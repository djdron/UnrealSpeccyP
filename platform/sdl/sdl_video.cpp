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

#include "../platform.h"

#ifdef USE_SDL

#include <SDL.h>
#include "../../ui/ui.h"

namespace xPlatform
{

static SDL_Surface* screen = NULL;
static SDL_Surface* offscreen = NULL;

bool InitVideo()
{
    screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE);
    if(!screen)
        return false;
	offscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 16,
						screen->format->Rmask,
						screen->format->Gmask,
						screen->format->Bmask,
						screen->format->Amask);
	if(!offscreen)
		return false;
	return true;
}
void DoneVideo()
{
	if(offscreen)
		SDL_FreeSurface(offscreen);
	if(screen)
		SDL_FreeSurface(screen);
}

inline word BGR565(byte r, byte g, byte b) { return ((r&~7) << 8)|((g&~3) << 3)|(b >> 3); }
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
			items[c] = BGR565(r, g, b);
			items_rgbx[c] = RGBX(r, g, b);
		}
	}
	word items[16];
	dword items_rgbx[16];
}
color_cache;

void UpdateScreen()
{
	SDL_LockSurface(offscreen);
	byte* data = (byte*)Handler()->VideoData();
	word* scr = (word*)offscreen->pixels;
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
			scr += offscreen->pitch - 320*2;
		}
	}
	else
#endif//USE_UI
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320/4; ++x)
			{
				*scr++ = color_cache.items[*data++];
				*scr++ = color_cache.items[*data++];
				*scr++ = color_cache.items[*data++];
				*scr++ = color_cache.items[*data++];
			}
			scr += offscreen->pitch - 320*2;
		}
	}
	SDL_UnlockSurface(offscreen);
	SDL_BlitSurface(offscreen, NULL, screen, NULL);
	SDL_Flip(screen);
}

}
//namespace xPlatform

#endif//USE_SDL
