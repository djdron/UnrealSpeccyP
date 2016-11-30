/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../platform.h"

#ifdef _ANDROID

#include "../../ui/ui.h"

namespace xPlatform
{

inline dword RGBX(byte r, byte g, byte b) { return (b << 16)|(g << 8)|r; }

static struct eCachedColors
{
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
			items_rgbx[c] = RGBX(r, g, b);
		}
	}
	dword items_rgbx[256];
}
color_cache;

void UpdateScreen(dword* scr)
{
	byte* data = (byte*)Handler()->VideoData();
#ifdef USE_UI
	byte* data_ui = (byte*)Handler()->VideoDataUI();
	if(data_ui)
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				xUi::eRGBAColor c_ui = xUi::palette[*data_ui++];
				xUi::eRGBAColor c = color_cache.items_rgbx[*data++];
				*scr++ = RGBX((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
		}
	}
	else
#endif//USE_UI
	{
		dword* cc = color_cache.items_rgbx;
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320/8; ++x)
			{
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
				*scr++ = cc[*data++];
			}
		}
	}
}

}
//namespace xPlatform

#endif//_ANDROID
