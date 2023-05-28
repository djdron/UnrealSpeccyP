/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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
#include "../../ui/ui.h"

#include "haiku_window.h"

namespace xPlatform
{

static HaikuPlatformWindow* window = NULL;
static bigtime_t lastRepaintTime;

static inline dword BGRX(byte r, byte g, byte b)
{
	return (r << 16) | (g << 8) | b;
}

static struct eCachedColors
{
	eCachedColors()
	{
		const byte brightness = 200;
		const byte bright_intensity = 55;
		for(int c = 0; c < 16; ++c) {
			byte i = c & 8 ? brightness + bright_intensity : brightness;
			byte b = c & 1 ? i : 0;
			byte r = c & 2 ? i : 0;
			byte g = c & 4 ? i : 0;
			items[c] = BGRX(r, g, b);
		}
	} dword items[16];
} color_cache;

bool InitVideo()
{
	window = new HaikuPlatformWindow(BRect(100, 100, 100 + 640, 100 + 515), xUi::WIDTH, xUi::HEIGHT,
		Handler()->WindowCaption(), B_TITLED_WINDOW, 0);
	window->Show();

	lastRepaintTime = system_time();

	return true;
}

void DoneVideo()
{
	window->Lock();
	window->Quit();
}

void UpdateScreen()
{
	if (system_time() - lastRepaintTime < 1000000 / 50)
		return;

	window->View()->LockBuffer();

	byte* data = (byte*)Handler()->VideoData();	
	dword* hscr = (dword*)window->View()->GetBuffer(); 
	byte* data_ui = (byte*)Handler()->VideoDataUI();

	if(data_ui)	{
		for(int y = 0; y < xUi::HEIGHT; ++y)
			for(int x = 0; x < xUi::WIDTH; ++x) {
				xUi::eRGBAColor c_ui = xUi::palette[*data_ui++];
				xUi::eRGBAColor c = color_cache.items[*data++];
				*hscr++ = BGRX((c.b >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.r >> c_ui.a) + c_ui.b);
			}
	} else {
		for(int y = 0; y < xUi::HEIGHT; ++y)
			for(int x = 0; x < xUi::WIDTH; ++x)
				*hscr++ = color_cache.items[*data++];
	}

	window->View()->UnlockBuffer();
	lastRepaintTime = system_time();
}

}
