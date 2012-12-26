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

#include "../platform.h"

#ifdef USE_SDL
#ifdef SDL_KEYS_PSP

#include <SDL.h>
#include "../../options_common.h"

#define PSP_BUTTON_UP            8
#define PSP_BUTTON_DOWN          6
#define PSP_BUTTON_RIGHT         9
#define PSP_BUTTON_LEFT          7
#define PSP_BUTTON_R             5
#define PSP_BUTTON_L             4
#define PSP_BUTTON_A             1
#define PSP_BUTTON_B             2
#define PSP_BUTTON_X             0
#define PSP_BUTTON_Y             3
#define PSP_BUTTON_SELECT        10
#define PSP_BUTTON_START         11

namespace xPlatform
{

static bool l_shift = false, r_shift = false, b_select = false, b_start = false;
static byte TranslateJoy(Uint8 _key, dword& _flags)
{
	bool ui_focused = Handler()->VideoDataUI();
	switch(_key)
	{
	case PSP_BUTTON_UP:		return 'u';
	case PSP_BUTTON_DOWN:	return 'd';
	case PSP_BUTTON_LEFT:	return 'l';
	case PSP_BUTTON_RIGHT:	return 'r';

	case PSP_BUTTON_A:		return 'f';
	case PSP_BUTTON_B:		return 'e';
	case PSP_BUTTON_X:		return '1';
	case PSP_BUTTON_Y:		return ' ';

	case PSP_BUTTON_SELECT:
		b_select = _flags&KF_DOWN;
		if(b_select && b_start)
			OPTION_GET(op_quit)->Set(true);
		return 'm';
	case PSP_BUTTON_START:
		b_start = _flags&KF_DOWN;
		if(b_select && b_start)
			OPTION_GET(op_quit)->Set(true);
		return 'k';

	case PSP_BUTTON_L:
		l_shift = _flags&KF_DOWN;
		if(!ui_focused)
			Handler()->OnAction(A_RESET);
		break;
	case PSP_BUTTON_R:
		r_shift = _flags&KF_DOWN;
		if(!ui_focused)
		{
			SAFE_CALL(OPTION_GET(op_sound_source))->Change();
		}
		break;

	default:
		break;
	}
	if(b_select && b_start)
		OPTION_GET(op_quit)->(true);
	return 0;
}

void ProcessKey(SDL_Event& e) {}

void ProcessJoy(SDL_Event& e)
{
	switch(e.type)
	{
	case SDL_JOYBUTTONDOWN:
		{
			dword flags = KF_DOWN|OpJoyKeyFlags();
			if(l_shift)
				flags |= KF_SHIFT;
			if(r_shift)
				flags |= KF_ALT;
			byte key = TranslateJoy(e.jbutton.button, flags);
			Handler()->OnKey(key, flags);
		}
		break;
	case SDL_JOYBUTTONUP:
		{
			dword flags = 0;
			if(l_shift)
				flags |= KF_SHIFT;
			if(r_shift)
				flags |= KF_ALT;
			byte key = TranslateJoy(e.jbutton.button, flags);
			Handler()->OnKey(key, OpJoyKeyFlags());
		}
		break;
	default:
		break;
	}
}

}
//namespace xPlatform

#endif//SDL_KEYS_PSP
#endif//USE_SDL
