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

#include "tui_keyboard.h"

#ifdef USE_TUI

#include "../platform.h"
#include "../../options_common.h"

namespace xPlatform
{

enum { KEY_COUNT = 40 };
static int key_state[KEY_COUNT];
static const char key_map[KEY_COUNT] =
{
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'e',
	'c', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', 's', ' ',
};

static const char* key_map2[] =
{
	"11111111222222223333333444444455555556666666677777778888888999999900000000000000",
	"QQQQQQQQQQQQWWWWWWWWEEEEEEERRRRRRRTTTTTTTYYYYYYYYUUUUUUUIIIIIIIOOOOOOOPPPPPPPPPP",
	"AAAAAAAAAAAAAASSSSSSSSDDDDDDDFFFFFFFGGGGGGGHHHHHHHHJJJJJJJKKKKKKKLLLLLLLeeeeeeee",
	"cccccccccccZZZZZZZXXXXXXXCCCCCCCVVVVVVVVBBBBBBBNNNNNNNMMMMMMMssssssss           ",
};

void OnTouchKey(float _x, float _y, bool down, int pointer_id)
{
	int pid = pointer_id + 1;
	int x = _x*80;
	int y = _y*4;
	if(x < 0 || x >= 80)
		return;
	if(y < 0 || y >= 4)
		return;

	using namespace xPlatform;
	dword flags = OpJoyKeyFlags();
	char c = key_map2[y][x];
	int k = 0;
	for(;k < KEY_COUNT; ++k)
	{
		if(c == key_map[k])
			break;
	}
	if(down) // release previous keys with this pid
	{
		for(int i = 0; i < KEY_COUNT; ++i)
		{
			if(key_state[i] == pid)
			{
				key_state[i] = 0;
				Handler()->OnKey(key_map[i], flags);
			}
		}
		key_state[k] = pid;
		flags |= KF_DOWN;
		if(key_state[30])
			flags |= KF_SHIFT;
		if(key_state[38])
			flags |= KF_ALT;
		Handler()->OnKey(key_map[k], flags);
	}
	else
	{
		key_state[k] = 0;
		Handler()->OnKey(key_map[k], flags);
	}
}

}
//namespace xPlatform

#endif//USE_TUI
