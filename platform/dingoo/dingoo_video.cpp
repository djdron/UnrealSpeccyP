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

struct eRayMirror : public xOptions::eOptionInt
{
	enum eState { S_FIRST, S_NONE = S_FIRST, S_H, S_V, S_HV, S_LAST };
	bool H() const { return value&S_H; }
	bool V() const { return value&S_V; }
	virtual const char* Name() const { return "ray mirror"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "n", "h", "v", "hv", NULL };
		return values;
	}
	virtual void Change(bool next = true) { eOptionInt::Change(S_LAST, next); }
};

struct eRaySync : public xOptions::eOptionInt
{
	enum eState { S_FIRST, S_OFF = S_FIRST, S_RAW, S_PERFECT, S_LAST };
	bool Perfect() const { return value == S_PERFECT; }
	virtual const char* Name() const { return "ray sync"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "raw", "perfect", NULL };
		return values;
	}
	virtual void Change(bool next = true) { eOptionInt::Change(S_LAST, next); }
};

struct eFrameRate : public xOptions::eOptionInt
{
	eFrameRate() { Set(13); }
	virtual const char* Name() const { return "frame rate"; }
	virtual void Change(bool next = true) { eOptionInt::Change(14, next); }
};

struct eFrameRate9325 : public eFrameRate
{
	virtual const char** Values() const
	{
		static const char* values[] =
		{
			"40", "43", "45", "48", "51", "55", "59", "64"
			,"70", "77", "85", "96", "110", "128", NULL
		};
		return values;
	}
};

struct eFrameRate9331 : public eFrameRate
{
	virtual const char** Values() const
	{
		static const char* values[] =
		{
			"30", "31", "33", "35", "38", "40", "43", "47"
			,"51", "56", "62", "70", "80", "93", NULL
		};
		return values;
	}
};

struct eVBlank : public xOptions::eOptionInt
{
	eVBlank() { Set(7); }
	virtual word Default() const = 0;
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
	virtual void Change(bool next = true) { eOptionInt::Change(15, next); }
};

struct eVBlank9325 : public eVBlank
{
	virtual word Default() const { return 0x0207; }
};

struct eVBlank9331 : public eVBlank
{
	virtual word Default() const { return 0x0202; }
};

class eLcd : public xOptions::eRootOption<xOptions::eOptionB>
{
public:
	virtual ~eLcd() {}
	virtual void Init() {}
	void Done()
	{
		if(ray_sync)
		{
			ray_sync.Set(eRaySync::S_OFF);
			Update();
		}
	}
	void Flip()
	{
		if(!ray_sync.Perfect())
			return;
		frame_counter &= 1;
		if(!frame_counter++)
		{
			Reset();
		}
	}
	const eRayMirror& RayMirror() const { return ray_mirror; }
	const eRaySync& RaySync() const { return ray_sync; }
	virtual const char* Name() const { return "video"; }
	virtual int Order() const { return 20; }
protected:
	eLcd() : frame_counter(0) {}
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
	virtual void Reset() = 0;
	virtual void Update() = 0;
	virtual void OnOption()
	{
		bool ray_mirror_changed = Option(ray_mirror);
		bool ray_sync_changed = Option(ray_sync);
		if(ray_mirror_changed || ray_sync_changed)
		{
			Update();
		}
		if(ray_sync.Perfect())
		{
			PerfectSyncOptions();
		}
	}
	virtual void PerfectSyncOptions() {}
protected:
	int frame_counter;
	eRayMirror ray_mirror;
	eRaySync ray_sync;
};
DECLARE_REGISTRATOR(eLcds, eLcd);

class eLcd9325 : public eLcd
{
public:
	virtual ~eLcd9325()
	{
		SAFE_DELETE(frame_rate);
		SAFE_DELETE(vblank);
	}
	virtual void Init()
	{
		frame_rate = new eFrameRate9325;
		vblank = new eVBlank9325;
	}
	virtual const char*	Value() const { return "ILI9325"; }
protected:
	virtual void Reset()
	{
		for(int i = 0; i < 5; ++i) //wait for lcd reset
			Set(R_RESET, (1 << 4)|(1 << 5)|(1 << 8));
		Set(R_RESET, 3|(1 << 4)|(1 << 5)|(1 << 8));
		Set(R_GRAM_WRITE);
	}
	virtual void Update()
	{
		bool mh = ray_mirror.H();
		bool mv = ray_mirror.V();
		word entry = ray_sync ? 0x1070 : 0x1048|(!mv ? 1 << 4 : 0)|(!mh ? 1 << 5 : 0);
		Set(R_ENTRY, entry);
		word vb = ((*vblank + 1) << 8)|(*vblank + 1);
		Set(R_VBLANK, ray_sync.Perfect() ? vb : vblank->Default());
		Set(R_FRAME_RATE, ray_sync.Perfect() ? *frame_rate : 0xd);
		Set(R_GRAM_HADDR, ray_sync || !mv ? 0 : 239);
		Set(R_GRAM_VADDR, ray_sync || !mh ? 0 : 319);
		Set(R_GRAM_WRITE);
	}
	virtual void PerfectSyncOptions()
	{
		bool frame_rate_changed = Option(frame_rate);
		bool vblank_changed = Option(vblank);
		if(frame_rate_changed || vblank_changed)
		{
			Update();
		}
	}
	enum eRegister { R_ENTRY = 0x03, R_RESET = 0x07, R_VBLANK = 0x08
		, R_GRAM_HADDR = 0x20, R_GRAM_VADDR = 0x21, R_GRAM_WRITE = 0x22, R_FRAME_RATE = 0x2b };
protected:
	eFrameRate* frame_rate;
	eVBlank* vblank;
};
REGISTER_TYPE(eLcds, eLcd9325, "9325");

class eLcd9331 : public eLcd9325
{
public:
	virtual void Init()
	{
		frame_rate = new eFrameRate9331;
		vblank = new eVBlank9331;
	}
	virtual const char* Value() const { return "ILI9331"; }
};
REGISTER_TYPE(eLcds, eLcd9331, "9331");

class eLcd9338 : public eLcd
{
public:
	virtual const char* Value() const { return "ILI9338"; }
protected:
	virtual void Reset()
	{
		for(int i = 0; i < 5; ++i) //wait for lcd reset
			Set(R_OFF);
		Set(R_ON);
		Set(R_MEMORY_WRITE);
	}
	virtual void Update()
	{
		bool mh = ray_mirror.H();
		bool mv = ray_mirror.V();
		word access = ray_sync ? 0x08 : 0x28|(mv ? 1 << 6 : 0)|(mh ? 1 << 7 : 0);
		Set(R_MEMORY_ACCESS, access);
		word vblank = ray_sync.Perfect() ? vblank : 0x02;
		Set(R_VBLANK, vblank, vblank, 0x0a, 0x14);
		word frame_rate = ray_sync.Perfect() ? 0x0112 : 0x0011;
		Set(R_FRAME_RATE, frame_rate >> 8, frame_rate&0xff);
		word column = ray_sync ? 239 : 319;
		word page = ray_sync ? 319 : 239;
		Set(R_COLUMN_ADDR, 0, 0, column >> 8, column&0xff);
		Set(R_PAGE_ADDR, 0, 0, page >> 8, page&0xff);
		Set(R_MEMORY_WRITE);
	}
	enum eRegister { R_OFF = 0x28, R_ON = 0x29, R_COLUMN_ADDR = 0x2a
		, R_PAGE_ADDR = 0x2b, R_MEMORY_WRITE = 0x2c, R_MEMORY_ACCESS = 0x36
		, R_FRAME_RATE = 0xb1, R_VBLANK = 0xb5 };
};
REGISTER_TYPE(eLcds, eLcd9338, "9338");

static class eVideo
{
public:
	eVideo();
	~eVideo() { SAFE_CALL(lcd)->Done(); SAFE_DELETE(lcd); }
	void Flip();
	void Update();
protected:
	inline dword BGR565(byte r, byte g, byte b) const { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
protected:
	word colors565[16];
	dword colors888[16];
	eLcd* lcd;
	word* frame;
} video;

eVideo::eVideo() : lcd(NULL), frame(NULL)
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
			lcd->Init();
			break;
		}
	}
	frame = (word*)_lcd_get_frame();
}
void eVideo::Flip()
{
	PROFILER_SECTION(flip);
	SAFE_CALL(lcd)->Flip();
	_lcd_set_frame();
}
void eVideo::Update()
{
	PROFILER_SECTION(draw);
	const byte* src = (const byte*)Handler()->VideoData();
	const byte* src_ui = (const byte*)Handler()->VideoDataUI();
	word* dst = frame;
	int ray_sync = lcd ? lcd->RaySync() : 0;
	bool mirr_h = ray_sync && lcd ? lcd->RayMirror().H() : 0;
	bool mirr_v = ray_sync && lcd ? lcd->RayMirror().V() : 0;
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
