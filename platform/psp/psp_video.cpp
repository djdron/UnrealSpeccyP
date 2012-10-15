/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef _PSP

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include "../platform.h"
#include "../../tools/profiler.h"
#include "../../tools/options.h"
#include "../../ui/ui.h"

PROFILER_DECLARE(u_vid0);
PROFILER_DECLARE(u_vid1);
PROFILER_DECLARE(u_vid2);
PROFILER_DECLARE(u_vid3);

namespace xPlatform
{

static struct eOptionZoom : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "zoom"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "fill screen", "small border", "no border", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(0, 4, next);
	}
	virtual int Order() const { return 35; }
} op_zoom;

struct eZoomVertex
{
	int ox, oy;
	int dx, dy;
};
static const eZoomVertex zoom_vertices[] = { { 80, 16, 400, 256 }, { 59, 0, 421, 272 }, { 33, -19, 447, 291 }, { 14, -34, 466, 306 } };

enum { BUF_WIDTH = 512, SCR_WIDTH = 480, SCR_HEIGHT = 272 };

static dword __attribute__((aligned(16))) gu_direct[262144];
static word  __attribute__((aligned(16))) clut[16];
static dword __attribute__((aligned(16))) clut_ui[8];
static byte* buffer = NULL; // allocated in vram
static byte* buffer_ui = NULL;

static struct eCachedColors
{
	inline dword BGR565(byte r, byte g, byte b) { return ((b&~7) << 8)|((g&~3) << 3)|(r >> 3); }
	eCachedColors()
	{
		const byte brightness = 200;
		const byte bright_intensity = 55;
		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? brightness + bright_intensity : brightness;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			clut[c] = BGR565(r, g, b);
		}
	}
}
color_cache;

void InitVideo()
{
	memcpy(clut_ui, xUi::palette, xUi::PALETTE_SIZE*sizeof(dword));
	sceGuInit();
	sceGuStart(GU_DIRECT, gu_direct);
	byte* fbp = NULL;
	byte* fbp0 = fbp;
	byte* fbp1 = fbp0 + 2*BUF_WIDTH*SCR_HEIGHT;
	byte* tp1 = fbp1 + 2*BUF_WIDTH*SCR_HEIGHT;
	buffer = tp1 + (ptrdiff_t)sceGeEdramGetAddr();
	buffer_ui = buffer + 512*256;
	sceGuDrawBuffer(GU_PSM_5650, fbp0, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, fbp1, BUF_WIDTH);
	sceGuDisable(GU_SCISSOR_TEST);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0, 0);
	sceDisplayWaitVblankStart();
	sceGuDisplay(1);
}

void DoneVideo()
{
	sceGuTerm();
}

struct Vertex
{
	word u, v;
	word color;
	word x, y, z;
};

void DrawSprite(const eZoomVertex& v)
{
	Vertex* vertices = (Vertex*)sceGuGetMemory(2 * sizeof(Vertex));
	vertices[0].u = 0;
	vertices[0].v = 0;
	vertices[0].color = 0;
	vertices[0].x = v.ox;
	vertices[0].y = v.oy;
	vertices[0].z = 0;

	vertices[1].u = 320;
	vertices[1].v = 240;
	vertices[1].color = 0;
	vertices[1].x = v.dx;
	vertices[1].y = v.dy;
	vertices[1].z = 0;

	sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT|GU_COLOR_4444|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 2, 0, vertices);
}

inline void swizzle_block(const byte* in, byte* out)
{
	const dword* din0 = (const dword*)in;
	dword* dout = (dword*)out;
	for(int y = 0; y < 8; ++y)
	{
		const dword* din = din0;
		*dout++ = *din++;
		*dout++ = *din++;
		*dout++ = *din++;
		*dout++ = *din;
		din0 += 320/sizeof(dword);
	}
}

void swizzle_320x240_to_512x256(const byte* in, byte* out)
{
	const byte* pin0 = in;
	for(int y = 0; y < 240; y += 8)
	{
		const byte* pin = pin0;
		for(int x = 0; x < 320; x += 16)
		{
			swizzle_block(pin, out);
			pin += 16;
			out += 16*8;
		}
		pin0 += 320*8;
		out += (512-320)/16*(8*16); //skip unused data
	}
}

void UpdateScreen()
{
	PROFILER_BEGIN(u_vid3);
	sceGuSync(0, 0);

	timeval tval;
	gettimeofday(&tval,0);
	qword time_current = tval.tv_sec * 1000000 + tval.tv_usec;
	static qword time_last = 0;
	qword dt = time_current - time_last;
	const qword dt_org = 16200; // ~1/60 sec
	if(dt < dt_org) // wait for vsync if we are too fast
	{
		sceDisplayWaitVblankStart();
//		sceKernelDelayThread(dt_org - dt);
		gettimeofday(&tval,0);
		time_current = tval.tv_sec * 1000000 + tval.tv_usec;
	}
	time_last = time_current;

//	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
	PROFILER_END(u_vid3);

	PROFILER_BEGIN(u_vid0);
	byte* data = (byte*)Handler()->VideoData();
	swizzle_320x240_to_512x256(data, buffer);
	sceKernelDcacheWritebackAll();
	PROFILER_END(u_vid0);

	PROFILER_BEGIN(u_vid1);
	sceGuStart(GU_DIRECT, gu_direct);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	sceGuClutMode(GU_PSM_5650, 0, 0x0f, 0);
	sceGuClutLoad(2, clut);
	sceGuTexMode(GU_PSM_T8, 0, 0, GU_TRUE);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGB);
	sceGuTexImage(0, 512, 256, 512, buffer);

	sceGuDisable(GU_BLEND);
	if(!op_zoom)
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	else
		sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	DrawSprite(zoom_vertices[op_zoom]);
	PROFILER_END(u_vid1);

	byte* data_ui = (byte*)Handler()->VideoDataUI();
	if(data_ui)
	{
		PROFILER_BEGIN(u_vid0);
		swizzle_320x240_to_512x256(data_ui, buffer_ui);
		sceKernelDcacheWritebackAll();
		PROFILER_END(u_vid0);

		PROFILER_BEGIN(u_vid2);
		sceGuClutMode(GU_PSM_8888, 0, 0x0f, 0);
		sceGuClutLoad(1, clut_ui);
		sceGuTexMode(GU_PSM_T8, 0, 0, GU_TRUE);
		sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
		sceGuTexImage(0, 512, 256, 512, buffer_ui);
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGuEnable(GU_BLEND);
		sceGuTexFilter(GU_NEAREST, GU_NEAREST);
		DrawSprite(zoom_vertices[0]);
		PROFILER_END(u_vid2);
	}
	sceGuFinish();
}

}
//namespace xPlatform

#endif//_PSP

