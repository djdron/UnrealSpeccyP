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
#ifdef SDL_KEYS_DINGOO

#include <SDL.h>
#include "../../options_common.h"

#define DINGOO_BUTTON_UP            SDLK_UP
#define DINGOO_BUTTON_DOWN          SDLK_DOWN
#define DINGOO_BUTTON_RIGHT         SDLK_RIGHT
#define DINGOO_BUTTON_LEFT          SDLK_LEFT
#define DINGOO_BUTTON_R             SDLK_BACKSPACE
#define DINGOO_BUTTON_L             SDLK_TAB
#define DINGOO_BUTTON_A             SDLK_LCTRL
#define DINGOO_BUTTON_B             SDLK_LALT
#define DINGOO_BUTTON_X             SDLK_SPACE
#define DINGOO_BUTTON_Y             SDLK_LSHIFT
#define DINGOO_BUTTON_SELECT        SDLK_ESCAPE
#define DINGOO_BUTTON_START         SDLK_RETURN

namespace xPlatform
{

static bool l_shift = false, r_shift = false, b_select = false, b_start = false;
static byte TranslateKey(SDLKey _key, dword& _flags)
{
	bool ui_focused = Handler()->VideoDataUI();
	switch(_key)
	{
	case DINGOO_BUTTON_UP:		return 'u';
	case DINGOO_BUTTON_DOWN:	return 'd';
	case DINGOO_BUTTON_LEFT:	return 'l';
	case DINGOO_BUTTON_RIGHT:	return 'r';

	case DINGOO_BUTTON_A:		return 'f';
	case DINGOO_BUTTON_B:		return 'e';
	case DINGOO_BUTTON_X:		return '1';
	case DINGOO_BUTTON_Y:		return ' ';

	case DINGOO_BUTTON_SELECT:
		b_select = _flags&KF_DOWN;
		if(b_select && b_start)
			OPTION_GET(op_quit)->Set(true);
		return 'm';
	case DINGOO_BUTTON_START:
		b_start = _flags&KF_DOWN;
		if(b_select && b_start)
			OPTION_GET(op_quit)->Set(true);
		return 'k';

	case DINGOO_BUTTON_L:
		l_shift = _flags&KF_DOWN;
		if(!ui_focused)
			Handler()->OnAction(A_RESET);
		break;
	case DINGOO_BUTTON_R:
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
		OPTION_GET(op_quit)->Set(true);
	return 0;
}

void ProcessKey(SDL_Event& e)
{
	switch(e.type)
	{
	case SDL_KEYDOWN:
		{
			dword flags = KF_DOWN|OpJoyKeyFlags();
			if(l_shift)
				flags |= KF_SHIFT;
			if(r_shift)
				flags |= KF_ALT;
			byte key = TranslateKey(e.key.keysym.sym, flags);
			Handler()->OnKey(key, flags);
		}
		break;
	case SDL_KEYUP:
		{
			dword flags = 0;
			if(l_shift)
				flags |= KF_SHIFT;
			if(r_shift)
				flags |= KF_ALT;
			byte key = TranslateKey(e.key.keysym.sym, flags);
			Handler()->OnKey(key, OpJoyKeyFlags());
		}
		break;
	default:
		break;
	}
}

}
//namespace xPlatform

#endif//SDL_KEYS_DINGOO
#endif//USE_SDL
