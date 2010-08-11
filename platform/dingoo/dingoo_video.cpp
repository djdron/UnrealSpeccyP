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

enum eRayMirror { RS_FIRST, RS_NONE = RS_FIRST, RS_MIRROR_H, RS_MIRROR_V, RS_MIRROR_HV, RS_LAST };

static struct eOptionRayMirror : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "ray mirror"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "hv", "v", "h", "n", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(RS_FIRST, RS_LAST, next);
	}
	virtual int Order() const { return 75; }
} op_ray_mirror;

static struct eOptionRaySync : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "ray sync"; }
	virtual int Order() const { return 76; }
} op_ray_sync;

class eVideo
{
public:
	eVideo() : frame(NULL), ray_mirror(0), ray_sync(false)
	{
		frame = (word*)_lcd_get_frame();
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
			RaySync(RaySyncRestoreEntry());
	}
	void Flip();
	void Update();
protected:
	void Set(dword cmd, int data = -1);
	enum { BRIGHTNESS = 190, BRIGHT_INTENSITY = 65 };
	inline dword BGR565(byte r, byte g, byte b) const { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
	void	UpdateRayOptions();
	void	RaySync(dword entry);
	dword	RaySyncRestoreEntry() const { return 0x1048|((((~ray_mirror&2) >> 1)|((~ray_mirror&1) << 1)) << 4); }
protected:
	word*	frame;
	word	colors565[16];
	dword	colors888[16];
	int		ray_mirror;
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
	_lcd_set_frame();
}
void eVideo::RaySync(dword entry)
{
	Set(0x2b, 0x000d);	//max refresh rate
	Set(0x03, entry);	//entry mode
	Set(0x20, 0);
	Set(0x21, 0);
	Set(0x22);			//write to GRAM
}
void eVideo::UpdateRayOptions()
{
	ray_mirror = 3 - op_ray_mirror;
	if(ray_sync != op_ray_sync)
	{
		ray_sync = op_ray_sync;
		RaySync(ray_sync ? 0x1070 : RaySyncRestoreEntry());
	}
}
void eVideo::Update()
{
	PROFILER_SECTION(draw);
	byte* src = (byte*)Handler()->VideoData();
	dword* src_ui = (dword*)Handler()->VideoDataUI();
	word* dst = frame;
	UpdateRayOptions();
	int mirr = ray_sync ? ray_mirror : 0;
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

void UpdateVideo()
{
	static eVideo video;
	video.Update();
	video.Flip();
}

}
//namespace xPlatform

#endif//_DINGOO
