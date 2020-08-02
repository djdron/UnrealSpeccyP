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
#include "../../tools/options.h"
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

#ifdef RG350 //map A-stick to movement
	static byte TranslateJoy(Uint8 _key2, dword& _flags)
	{
	        switch(_key2)
	        {
	        case 1:     return 'u';
	        case 2:   	return 'd';
	        case 3:   	return 'l';
	        case 4:  	return 'r';
	
	        default:
	                break;
	        }
		return 0;
	}
#endif //RG350

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
			OpQuit(true);
		return 'm';
	case DINGOO_BUTTON_START:
		b_start = _flags&KF_DOWN;
		if(b_select && b_start)
			OpQuit(true);
		return 'k';

	case DINGOO_BUTTON_L:
		l_shift = _flags&KF_DOWN;
		if(!ui_focused)
		{
		#ifdef RG350 //redefine L as save state
	        using namespace xOptions;
	        eOptionB* o = eOptionB::Find("save state");
	        SAFE_CALL(o)->Change();
		#else
			Handler()->OnAction(A_RESET);
		#endif//RG350
        }
		break;
	case DINGOO_BUTTON_R:
		r_shift = _flags&KF_DOWN;
		if(!ui_focused)
		{
		#ifdef RG350 //redefine R as load state
			using namespace xOptions;
			eOptionB* o = eOptionB::Find("load state");
			SAFE_CALL(o)->Change();
		#else
			xOptions::eOption<int>* op_sound = xOptions::eOption<int>::Find("sound");
			SAFE_CALL(op_sound)->Change();
		#endif//RG350
		}
		break;

	default:
		break;
	}
	if(b_select && b_start)
		OpQuit(true);
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

#ifdef RG350//A-stick code, mostly ripped from the wiki ;)
	void ProcessJoy(SDL_Event& e)
	{
		int JOY_DEADZONE=1000;
        	switch(e.type)
		{
			case SDL_JOYAXISMOTION:			
			switch(e.jaxis.axis)
			{
				case 0:		// axis 0 (left-right)
				if(e.jaxis.value < -JOY_DEADZONE)
				{
					dword flags = KF_DOWN|OpJoyKeyFlags();
					if(l_shift)
						flags |= KF_SHIFT;
					if(r_shift)
						flags |= KF_ALT;
					byte key = TranslateJoy(3, flags);
					Handler()->OnKey(key, flags);
				}
				else if(e.jaxis.value > JOY_DEADZONE)
				{
					dword flags = KF_DOWN|OpJoyKeyFlags();
					if(l_shift)
						flags |= KF_SHIFT;
					if(r_shift)
						flags |= KF_ALT;
					byte key = TranslateJoy(4, flags);
					Handler()->OnKey(key, flags);
				} else //A-stick within deadzone (centred)
	                	{
        	        	        dword flags = 0;
        	        	        if(l_shift)
        	        	                flags |= KF_SHIFT;
        	        	        if(r_shift)
        	        	                flags |= KF_ALT;
        	        	        byte key = TranslateJoy(3, flags);
        	        	        byte key2 = TranslateJoy(4, flags);
        	        	        Handler()->OnKey(key, OpJoyKeyFlags());
        	        	        Handler()->OnKey(key2, OpJoyKeyFlags());
        	        	}

				break;

				case 1:		// axis 1 (up-down)
				if(e.jaxis.value < -JOY_DEADZONE)
				{
						// up movement
					dword flags = KF_DOWN|OpJoyKeyFlags();
					if(l_shift)
						flags |= KF_SHIFT;
					if(r_shift)
						flags |= KF_ALT;
					byte key = TranslateJoy(1, flags);
					Handler()->OnKey(key, flags);
				}
				else if(e.jaxis.value > JOY_DEADZONE)
				{
						// down movement
					dword flags = KF_DOWN|OpJoyKeyFlags();
					if(l_shift)
						flags |= KF_SHIFT;
					if(r_shift)
						flags |= KF_ALT;
					byte key = TranslateJoy(2, flags);
					Handler()->OnKey(key, flags);
				} else //A-stick within deadzone (centred)
	                	{
        	                	dword flags = 0;
        	                	if(l_shift)
                	        	        flags |= KF_SHIFT;
                	        	if(r_shift)
                	        	        flags |= KF_ALT;
                	        	byte key = TranslateJoy(1, flags);
                	        	Handler()->OnKey(key, OpJoyKeyFlags());
                	        	byte key2 = TranslateJoy(2, flags);
                	        	Handler()->OnKey(key2, OpJoyKeyFlags());
                		}

				break;

				default: //invoked if A-stick exactly centred
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
			}
		}
	}
#endif//RG350
}
//namespace xPlatform

#endif//SDL_KEYS_DINGOO
#endif//USE_SDL
