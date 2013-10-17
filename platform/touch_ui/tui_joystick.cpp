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

#include "tui_joystick.h"

#ifdef USE_TUI

#include "../platform.h"
#include "../../options_common.h"

namespace xPlatform
{

enum { KEY_COUNT = 60 };
static int key_state[KEY_COUNT];
static const char key_map[KEY_COUNT] =
{
	'L', 'L', 'u', 'U', 'U', ' ', ' ', ' ', 'e', 'e',
	'L', 'L', 'u', 'U', 'U', '.', ' ', ' ', 'f', '.',
	'l', 'l', '.', 'r', 'r', '.', 'f', 'f', 'f', 'f',
	'D', 'D', 'd', 'R', 'R', '.', 'f', 'f', 'f', 'f',
	'D', 'D', 'd', 'R', 'R', '.', 'f', 'f', 'f', 'f',
	'D', 'D', 'd', 'R', 'R', '.', 'f', 'f', 'f', 'f',
};

static void ProcessKey(char key, dword flags)
{
	switch(key)
	{
	case 'L':
		Handler()->OnKey('u', flags);
		Handler()->OnKey('l', flags);
		break;
	case 'U':
		Handler()->OnKey('u', flags);
		Handler()->OnKey('r', flags);
		break;
	case 'R':
		Handler()->OnKey('d', flags);
		Handler()->OnKey('r', flags);
		break;
	case 'D':
		Handler()->OnKey('d', flags);
		Handler()->OnKey('l', flags);
		break;
	default:
		Handler()->OnKey(key, flags);
		break;
	}
}

void OnTouchJoy(float _x, float _y, bool down, int pointer_id)
{
	int pid = pointer_id + 1;
	int x = _x*10;
	int y = _y*5.625f;
	if(x < 0 || x >= 10)
		return;
	if(y < 0 || y >= 6)
		return;

	using namespace xPlatform;
	dword flags = OpJoyKeyFlags();
	int k = y*10+x;
	if(down) // release previous keys with this pid
	{
		for(int i = 0; i < KEY_COUNT; ++i)
		{
			if(key_state[i] == pid)
			{
				key_state[i] = 0;
				ProcessKey(key_map[i], flags);
			}
		}
		if(key_map[k] == '.')
			return;
		key_state[k] = pid;
		flags |= KF_DOWN;
		ProcessKey(key_map[k], flags);
	}
	else
	{
		if(key_map[k] == '.')
			return;
		key_state[k] = 0;
		ProcessKey(key_map[k], flags);
	}
}

}
//namespace xPlatform

#endif//USE_TUI
