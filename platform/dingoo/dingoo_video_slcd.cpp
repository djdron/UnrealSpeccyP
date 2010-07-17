/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _DINGOO

#include "../platform.h"
#include "../../ui/ui.h"
#include "../../tools/options.h"
#include "../../tools/profiler.h"

#include <dingoo/jz4740.h>
#include <dingoo/cache.h>
#include <dingoo/slcd.h>

PROFILER_DECLARE(draw);
PROFILER_DECLARE(flip);

void* g_pGameDecodeBuf = NULL;

namespace xPlatform
{

enum eRaySync { RS_FIRST, RS_OFF = RS_FIRST, RS_ON, RS_MIRROR_H, RS_MIRROR_V, RS_MIRROR_HV, RS_LAST };

static struct eOptionRaySync : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "ray sync"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "on", "mirror_h", "mirror_v", "mirror_hv", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(RS_FIRST, RS_LAST, next);
	}
	virtual int Order() const { return 75; }
} op_ray_sync;

class eVideo
{
public:
	eVideo() : frame(NULL), frame_back(NULL), ray_sync(false)
	{
		frame = (word*)_lcd_get_frame();
		frame_back = (word*)g_pGameDecodeBuf;

		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? BRIGHTNESS + BRIGHT_INTENSITY : BRIGHTNESS;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			colors888[c] = xUi::eRGBAColor(r, g, b).rgba;
			colors565[c] = BGR565(r, g, b);
		}
	}
	~eVideo()
	{
		if(ray_sync)
		{
			int mirr = op_ray_sync;
			mirr = mirr ? mirr - 1 : 0;
			Set(0x03, 0x1048|(mirr&3 << 4)); //entry mode restore
			Set(0x2b, 0x000d);	//default refresh rate
			Set(0x20, 0);
			Set(0x21, 0);
			Set(0x22);			//write to GRAM
		}
	}
	void Flip();
	void Update();
protected:
	void Set(dword cmd, int data = -1);
	enum { BRIGHTNESS = 190, BRIGHT_INTENSITY = 65 };
	inline dword BGR565(byte r, byte g, byte b) const { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
protected:
	word*	frame;
	word*	frame_back;
	word	colors565[16];
	dword	colors888[16];
	bool	ray_sync;
};

void eVideo::Set(dword cmd, int data)
{
	// SLCD related GPIO pins for the Dingoo A320
	const int pin_rs_n = 32*2+19;	// port 2 pin 19: RS# (register select, active low)

	__gpio_clear_pin(pin_rs_n);
	REG_SLCD_DATA = SLCD_DATA_RS_COMMAND|(cmd&0xffff);
	while(REG_SLCD_STATE&SLCD_STATE_BUSY);
	__gpio_set_pin(pin_rs_n);
	if(data == -1)
		return;
	REG_SLCD_DATA = SLCD_DATA_RS_DATA|(data&0xffff);
	while(REG_SLCD_STATE&SLCD_STATE_BUSY);
}
void eVideo::Flip()
{
	PROFILER_SECTION(flip);
	__dcache_writeback_all();
	if(ray_sync)
	{
		word* tmp = frame;
		frame = frame_back;
		frame_back = tmp;

		while(!(REG_DMAC_DCCSR(0)&DMAC_DCCSR_TT));	// Wait for transfer terminated bit
		REG_SLCD_CTRL = 1;							// Enable DMA on the SLCD
		REG_DMAC_DCCSR(0) = 0;						// Disable DMA channel while configuring
		REG_DMAC_DRSR(0) = DMAC_DRSR_RS_SLCD;		// DMA request source is SLCD
		REG_DMAC_DSAR(0) = (dword)frame&0x1fffffff;	// Set source, target and count
		REG_DMAC_DTAR(0) = SLCD_FIFO&0x1fffffff;
		REG_DMAC_DTCR(0) = (320*240*2)/16;

		// Source address increment, source width 32 bit,
		// destination width 16 bit, data unit size 16 bytes,
		// block transfer mode, no interrupt
		REG_DMAC_DCMD(0) = DMAC_DCMD_SAI|DMAC_DCMD_SWDH_32
			|DMAC_DCMD_DWDH_16|DMAC_DCMD_DS_16BYTE|DMAC_DCMD_TM;

		REG_DMAC_DCCSR(0) |= DMAC_DCCSR_NDES;		// No DMA descriptor used
		REG_DMAC_DCCSR(0) |= DMAC_DCCSR_EN;			// Set enable bit to start DMA
	}
	else
	{
		_lcd_set_frame();
	}
}
void eVideo::Update()
{
	PROFILER_SECTION(draw);
	byte* src = (byte*)Handler()->VideoData();
	dword* src_ui = (dword*)Handler()->VideoDataUI();
	word* dst = ray_sync ? frame_back : frame;
	int mirr = op_ray_sync;
	if(mirr && !ray_sync)
	{
		Set(0x2b, 0x000d);	//max refresh rate
		Set(0x20, 0);
		Set(0x21, 0);
		Set(0x22);			//write to GRAM
		Set(0x03, 0x1070);	//entry mode default
		Set(0x20, 0);
		Set(0x21, 0);
		Set(0x22);			//write to GRAM
		ray_sync = true;
	}
	mirr = mirr ? mirr - 1 : 0;
	bool mirr_h = mirr & 1;
	bool mirr_v = mirr & 2;
	int offs_base = !ray_sync ? 0 : mirr_h ? 319 : 0;
	int ext_step = !ray_sync ? 320 : mirr_h ? -1 : 1;
	int int_step = !ray_sync ? 1 : mirr_v ? -320 : 320;
	int ext_end = !ray_sync ? 320*240 : mirr_h ? -1 : 320;
	int int_end = !ray_sync ? 320 : mirr_v ? -320 : 320*240;
	offs_base += mirr_v ? 320*239 : 0;
	ext_end += mirr_v ? 320*239 : 0;
	int_end += mirr_h ? 319 : 0;
	if(src_ui)
	{
		for(; offs_base != ext_end; offs_base += ext_step, int_end += ext_step)
		{
			for(int offs = offs_base; offs != int_end; offs += int_step)
			{
				xUi::eRGBAColor c_ui = src_ui[offs];
				xUi::eRGBAColor c = colors888[src[offs]];
				*dst++ = BGR565((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
		}
	}
	else
	{
		for(; offs_base != ext_end; offs_base += ext_step, int_end += ext_step)
		{
			for(int offs = offs_base; offs != int_end; offs += int_step)
			{
				*dst++ = colors565[src[offs]];
			}
		}
	}
}

static eVideo& Video() { static eVideo video; return video; }

void UpdateVideo() { Video().Update(); }
void FlipVideo() { Video().Flip(); }

}
//namespace xPlatform

#endif//_DINGOO
