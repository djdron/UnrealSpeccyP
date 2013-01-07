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
#ifdef SDL_KEYS_COMMON

#include <SDL.h>
#include "../../tools/options.h"
#include "../../options_common.h"

namespace xPlatform
{

static bool ProcessFuncKey(SDL_Event& e)
{
	if(e.key.keysym.mod)
		return false;
	switch(e.key.keysym.sym)
	{
	case SDLK_F2:
		{
			using namespace xOptions;
			eOptionB* o = eOptionB::Find("save state");
			SAFE_CALL(o)->Change();
		}
		return true;
	case SDLK_F3:
		{
			using namespace xOptions;
			eOptionB* o = eOptionB::Find("load state");
			SAFE_CALL(o)->Change();
		}
		return true;
	case SDLK_F5:
		Handler()->OnAction(A_TAPE_TOGGLE);
		return true;
	case SDLK_F7:
		{
			static bool paused = false;
			paused = !paused;
			Handler()->VideoPaused(paused);
		}
		return true;
	case SDLK_F12:
		Handler()->OnAction(A_RESET);
		return true;
	default:
		return false;
	}
}

static byte TranslateKey(SDLKey _key, dword& _flags)
{
	switch(_key)
	{
	case SDLK_ESCAPE:	return 'm';
	case SDLK_LSHIFT:	return 'c';
	case SDLK_RSHIFT:	return 'c';
	case SDLK_LALT:		return 's';
	case SDLK_RALT:		return 's';
	case SDLK_RETURN:	return 'e';
	case SDLK_BACKQUOTE: return 'p';
	case SDLK_BACKSPACE:
		_flags |= KF_SHIFT;
		return '0';
	case SDLK_QUOTE:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return 'P';
		}
		else
			return '7';
	case SDLK_COMMA:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return 'R';
		}
		else
			return 'N';
	case SDLK_PERIOD:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return 'T';
		}
		else
			return 'M';
	case SDLK_SEMICOLON:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return 'Z';
		}
		else
			return 'O';
	case SDLK_SLASH:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return 'C';
		}
		else
			return 'V';
	case SDLK_MINUS:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return '0';
		}
		else
			return 'J';
	case SDLK_EQUALS:
		_flags |= KF_ALT;
		if(_flags&KF_SHIFT)
		{
			_flags &= ~KF_SHIFT;
			return 'K';
		}
		else
			return 'L';
	case SDLK_TAB:
		_flags |= KF_ALT;
		_flags |= KF_SHIFT;
		return 0;
	case SDLK_LEFT:		return 'l';
	case SDLK_RIGHT:	return 'r';
	case SDLK_UP:		return 'u';
	case SDLK_DOWN:		return 'd';
	case SDLK_INSERT:
	case SDLK_RCTRL:
	case SDLK_LCTRL:	return 'f';
	default:
		break;
	}
	if(_key >= SDLK_0 && _key <= SDLK_9)
		return _key;
	if(_key >= SDLK_a && _key <= SDLK_z)
		return toupper(_key);
	if(_key == SDLK_SPACE)
		return _key;
	return 0;
}

void ProcessKey(SDL_Event& e)
{
	switch(e.type)
	{
	case SDL_KEYDOWN:
		{
			dword flags = KF_DOWN|OpJoyKeyFlags();
			if(e.key.keysym.mod&KMOD_ALT)
				flags |= KF_ALT;
			if(e.key.keysym.mod&KMOD_SHIFT)
				flags |= KF_SHIFT;
			byte key = TranslateKey(e.key.keysym.sym, flags);
			Handler()->OnKey(key, flags);
		}
		break;
	case SDL_KEYUP:
		if(!ProcessFuncKey(e))
		{
			dword flags = 0;
			if(e.key.keysym.mod&KMOD_ALT)
				flags |= KF_ALT;
			if(e.key.keysym.mod&KMOD_SHIFT)
				flags |= KF_SHIFT;
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

#endif//SDL_KEYS_COMMON
#endif//USE_SDL
