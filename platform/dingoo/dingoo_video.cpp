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
#include "../../tools/type_registrator.h"

#include <dingoo/jz4740.h>
#include <dingoo/cache.h>
#include <dingoo/slcd.h>

PROFILER_DECLARE(draw);
PROFILER_DECLARE(flip);

void* g_pGameDecodeBuf = NULL;
extern "C" void* sys_get_ccpmp_config();

namespace xPlatform
{

enum eRaySync { RS_FIRST, RS_OFF = RS_FIRST, RS_RAW, RS_PERFECT, RS_LAST };

struct eOptionRaySync : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "ray sync"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "raw", "perfect", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(RS_FIRST, RS_LAST, next);
	}
	virtual int Order() const { return 75; }
};

enum eRayMirror { RM_FIRST, RM_NONE = RM_FIRST, RM_H, RM_V, RM_HV, RM_LAST };

struct eOptionRayMirror : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "ray mirror"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "n", "h", "v", "hv", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(RM_FIRST, RM_LAST, next);
	}
	virtual int Order() const { return 76; }
};

struct eOptionFrameRate : public xOptions::eOptionInt
{
	eOptionFrameRate() { Set(8); }
	virtual const char* Name() const { return "frame rate"; }
	virtual const char** Values() const
	{
		static const char* values[] =
		{
			"30", "31", "33", "35", "38", "40", "43", "47"
			,"51", "56", "62", "70", "80", "93", NULL
		};
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(0, 14, next);
	}
	virtual int Order() const { return 77; }
};

struct eOptionVBlank : public xOptions::eOptionInt
{
	eOptionVBlank() { Set(7); }
	virtual const char* Name() const { return "vblank"; }
	virtual const char** Values() const
	{
		static const char* values[] =
		{
			"1", "2", "3", "4", "5", "6", "7", "8"
			, "9", "a", "b", "c", "d", "e", "f", NULL
		};
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(0, 15, next);
	}
	virtual int Order() const { return 78; }
};

class eLcd
{
public:
	eLcd() : ray_sync(RS_OFF), ray_mirror(RM_NONE), frame_rate(0), vblank(0) {}
	virtual void Reset() = 0;
	virtual void Update() = 0;

	void RaySync(eRaySync v) { ray_sync = v; }
	void RayMirror(eRayMirror v) { ray_mirror = v; }
	void FrameRate(int v) { frame_rate = v; }
	void VBlank(int v) { vblank = v; }
	eRaySync RaySync() const{ return ray_sync; }
	eRayMirror RayMirror() const { return ray_mirror; }
	int FrameRate() const { return frame_rate; }
	int VBlank() const { return vblank; }
protected:
	void Set(word reg, int v1 = -1, int v2 = -1, int v3 = -1, int v4 = -1)
	{
		enum { PIN_RS_N = 32 * 2 + 19 };
		__gpio_clear_pin(PIN_RS_N);
		REG_SLCD_DATA = SLCD_DATA_RS_COMMAND|reg;
		while(REG_SLCD_STATE&SLCD_STATE_BUSY);
		__gpio_set_pin(PIN_RS_N);
		const int* vs[] = { &v1, &v2, &v3, &v4 };
		for(int i = 0; i < 4 && *vs[i] != -1; ++i)
		{
			REG_SLCD_DATA = SLCD_DATA_RS_DATA|(word)*vs[i];
			while(REG_SLCD_STATE&SLCD_STATE_BUSY);
		}
	}
protected:
	eRaySync ray_sync;
	eRayMirror ray_mirror;
	int frame_rate;
	int vblank;
};
DECLARE_REGISTRATOR(eLcds, eLcd);

class eLcd9325 : public eLcd
{
public:
	virtual void Reset()
	{
		for(int i = 0; i < 5; ++i) //wait for lcd reset
			Set(R_RESET, (1 << 4)|(1 << 5)|(1 << 8));
		Set(R_RESET, 3|(1 << 4)|(1 << 5)|(1 << 8));
		Set(R_GRAM_WRITE);
	}
	virtual void Update()
	{
		bool mh = ray_mirror&RM_H;
		bool mv = ray_mirror&RM_V;
		word entry = ray_sync ? 0x1070 : 0x1048|(!mv ? 1 << 4 : 0)|(!mh ? 1 << 5 : 0);
		Set(R_ENTRY, entry);
		word vb = ((vblank + 1) << 8)|(vblank + 1);
		Set(R_VBLANK, ray_sync == RS_PERFECT ? vb : VBlankDefault());
		Set(R_FRAME_RATE, ray_sync == RS_PERFECT ? frame_rate : 0xd);
		Set(R_GRAM_HADDR, ray_sync || !mv ? 0 : 239);
		Set(R_GRAM_VADDR, ray_sync || !mh ? 0 : 319);
		Set(R_GRAM_WRITE);
	}
protected:
	virtual word VBlankDefault() const { return 0x0207; }
	enum eRegister { R_ENTRY = 0x03, R_RESET = 0x07, R_VBLANK = 0x08
		, R_GRAM_HADDR = 0x20, R_GRAM_VADDR = 0x21, R_GRAM_WRITE = 0x22, R_FRAME_RATE = 0x2b };
};
REGISTER_TYPE(eLcds, eLcd9325, "9325");

class eLcd9331 : public eLcd9325
{
protected:
	virtual word VBlankDefault() const { return 0x0202; }
};
REGISTER_TYPE(eLcds, eLcd9331, "9331");

class eLcd9338 : public eLcd
{
public:
	virtual void Reset()
	{
		for(int i = 0; i < 5; ++i) //wait for lcd reset
			Set(R_OFF);
		Set(R_ON);
		Set(R_MEMORY_WRITE);
	}
	virtual void Update()
	{
		bool mh = ray_mirror&RM_H;
		bool mv = ray_mirror&RM_V;
		word access = ray_sync ? 0x08 : 0x28|(mv ? 1 << 6 : 0)|(mh ? 1 << 7 : 0);
		Set(R_MEMORY_ACCESS, access);
		word vblank = ray_sync == RS_PERFECT ? vblank : 0x0202;
		Set(R_VBLANK, vblank >> 8, vblank&0xff, 0x0a, 0x14);
		word frame_rate = ray_sync == RS_PERFECT ? 0x0112 : 0x0011;
		Set(R_FRAME_RATE, frame_rate >> 8, frame_rate&0xff);
		word column = ray_sync ? 239 : 319;
		word page = ray_sync ? 319 : 239;
		Set(R_COLUMN_ADDR, 0, 0, column >> 8, column&0xff);
		Set(R_PAGE_ADDR, 0, 0, page >> 8, page&0xff);
		Set(R_MEMORY_WRITE);
	}
protected:
	enum eRegister { R_OFF = 0x28, R_ON = 0x29, R_COLUMN_ADDR = 0x2a
		, R_PAGE_ADDR = 0x2b, R_MEMORY_WRITE = 0x2c, R_MEMORY_ACCESS = 0x36
		, R_FRAME_RATE = 0xb1, R_VBLANK = 0xb5 };
};
REGISTER_TYPE(eLcds, eLcd9338, "9338");

static class eVideo
{
public:
	eVideo();
	~eVideo();
	void Flip();
	void Update();
protected:
	inline dword BGR565(byte r, byte g, byte b) const { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
	void UpdateOptions();
protected:
	word colors565[16];
	dword colors888[16];
	int frame_counter;
	eLcd* lcd;
	word* frame;
	eOptionRaySync* op_ray_sync;
	eOptionRayMirror* op_ray_mirror;
	eOptionFrameRate* op_frame_rate;
	eOptionVBlank* op_vblank;
} video;

eVideo::eVideo() : frame_counter(0), lcd(NULL), frame(NULL)
	, op_ray_sync(NULL), op_ray_mirror(NULL), op_frame_rate(NULL), op_vblank(NULL)
{
	for(int c = 0; c < 16; ++c)
	{
		enum { BRIGHTNESS = 190, BRIGHT_INTENSITY = 65 };
		byte i = c&8 ? BRIGHTNESS + BRIGHT_INTENSITY : BRIGHTNESS;
		byte b = c&1 ? i : 0;
		byte r = c&2 ? i : 0;
		byte g = c&4 ? i : 0;
		colors888[c] = xUi::eRGBAColor(r, g, b).rgba;
		colors565[c] = BGR565(r, g, b);
	}
	enum { LCD_NAME_OFFSET = 72 };
	const char* lcd_name = (const char*)sys_get_ccpmp_config() + LCD_NAME_OFFSET;
	for(int i = 0; i < 32; ++i)
	{
		char lcd_id[] = "9325";
		strncpy(lcd_id, &lcd_name[i], 4);
		lcd = eLcds::Create(lcd_id);
		if(lcd)
		{
			op_ray_sync = new eOptionRaySync;
			op_ray_mirror = new eOptionRayMirror;
			op_frame_rate = new eOptionFrameRate;
			op_vblank = new eOptionVBlank;
			break;
		}
	}
	frame = (word*)_lcd_get_frame();
}
eVideo::~eVideo()
{
	if(lcd && *op_ray_sync)
	{
		lcd->RaySync(RS_OFF);
		lcd->Update();
	}
	SAFE_DELETE(lcd);
	SAFE_DELETE(op_ray_sync);
	SAFE_DELETE(op_ray_mirror);
	SAFE_DELETE(op_frame_rate);
	SAFE_DELETE(op_vblank);
}
void eVideo::Flip()
{
	PROFILER_SECTION(flip);
	if(lcd && *op_ray_sync == RS_PERFECT)
	{
		frame_counter &= 1;
		if(!frame_counter++)
		{
			SAFE_CALL(lcd)->Reset();
		}
	}
	_lcd_set_frame();
}
void eVideo::UpdateOptions()
{
	if(!lcd || (*op_ray_sync == lcd->RaySync() && *op_ray_mirror == lcd->RayMirror()
		&& *op_frame_rate == lcd->FrameRate() && *op_vblank == lcd->VBlank()))
		return;
	lcd->RaySync((eRaySync)(int)*op_ray_sync);
	lcd->RayMirror((eRayMirror)(int)*op_ray_mirror);
	lcd->FrameRate(*op_frame_rate);
	lcd->VBlank(*op_vblank);
	lcd->Update();
}
void eVideo::Update()
{
	PROFILER_SECTION(draw);
	const byte* src = (const byte*)Handler()->VideoData();
	const byte* src_ui = (const byte*)Handler()->VideoDataUI();
	word* dst = frame;
	UpdateOptions();
	int ray_sync = *op_ray_sync;
	int mirr = ray_sync ? *op_ray_mirror : 0;
	bool mirr_h = mirr&RM_H;
	bool mirr_v = mirr&RM_V;
	int offs_base = mirr_h ? 319 : 0;
	int ext_step = !ray_sync ? 320 : mirr_h ? -1 : 1;
	int int_step = !ray_sync ? 1 : mirr_v ? -320 : 320;
	int ext_end = !ray_sync ? 320*240 : mirr_h ? -1 : 320;
	int int_end = !ray_sync ? 320 : mirr_v ? -320 : 320*240;
	offs_base += mirr_v ? 320*239 : 0;
	ext_end += mirr_v ? 320*239 : 0;
	int_end += mirr_h ? 319 : 0;
	if(!src_ui)
	{
		for(; offs_base != ext_end; offs_base += ext_step, int_end += ext_step)
		{
			for(int offs = offs_base; offs != int_end; offs += int_step)
			{
				*dst++ = colors565[src[offs]];
			}
		}
		return;
	}
	for(; offs_base != ext_end; offs_base += ext_step, int_end += ext_step)
	{
		for(int offs = offs_base; offs != int_end; offs += int_step)
		{
			xUi::eRGBAColor c_ui = xUi::palette[src_ui[offs]];
			xUi::eRGBAColor c = colors888[src[offs]];
			*dst++ = BGR565((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
		}
	}
}

void UpdateVideo()
{
	while(!__dmac_channel_transmit_end_detected(0));
	video.Update();
	__dcache_writeback_all();
}
void FlipVideo() { video.Flip(); }

}
//namespace xPlatform

#endif//_DINGOO
