/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

inline dword BGR565(byte r, byte g, byte b) { return ((r&~7) << 8)|((g&~3) << 3)|(b >> 3); }
inline dword RGBX(byte r, byte g, byte b) { return (b << 16)|(g << 8)|r; }

static struct eCachedColors
{
	eCachedColors()
	{
		const byte brightness = 200;
		const byte bright_intensity = 55;
		for(int c1 = 0; c1 < 16; ++c1)
		{
			byte i1 = c1&8 ? brightness + bright_intensity : brightness;
			byte b1 = c1&1 ? i1 : 0;
			byte r1 = c1&2 ? i1 : 0;
			byte g1 = c1&4 ? i1 : 0;
			for(int c = 0; c < 16; ++c)
			{
				byte i = c&8 ? brightness + bright_intensity : brightness;
				byte b = c&1 ? i : 0;
				byte r = c&2 ? i : 0;
				byte g = c&4 ? i : 0;
				int x = c1*16 + c;
				items[x] = BGR565(r, g, b) | (BGR565(r1, g1, b1) << 16);
				items_shifted[x] = BGR565(r, g, b) << 16;
				items_rgbx[x] = RGBX(r, g, b);
			}
		}
	}
	dword items[256];
	dword items_shifted[256];
	dword items_rgbx[256];
}
color_cache;

void UpdateScreen(word* scr)
{
	byte* data = (byte*)Handler()->VideoData();
#ifdef USE_UI
	dword* data_ui = (dword*)Handler()->VideoDataUI();
	if(data_ui)
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				xUi::eRGBAColor c_ui = *data_ui++;
				xUi::eRGBAColor c = color_cache.items_rgbx[*data++];
				*scr++ = BGR565((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
		}
	}
	else
#endif//USE_UI
	{
		dword* scr1 = (dword*)scr;
		dword* data1 = (dword*)data;
		dword* cc = color_cache.items;
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320/8; ++x)
			{
				dword c1 = *data1++;
				dword c2 = *data1++;
				*scr1++ = cc[((c1&0x00000f00) >> 4)		| (c1&0x0000000f)];
				*scr1++ = cc[((c1&0x0f000000) >> 20)	| ((c1&0x000f0000) >> 16)];
				*scr1++ = cc[((c2&0x00000f00) >> 4)		| (c2&0x0000000f)];
				*scr1++ = cc[((c2&0x0f000000) >> 20)	| ((c2&0x000f0000) >> 16)];
			}
		}
	}
}

}
//namespace xPlatform

#endif//_ANDROID
