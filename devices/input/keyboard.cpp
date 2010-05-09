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

#include "../../std.h"
#include "keyboard.h"

//=============================================================================
//	eKeyboard::Init
//-----------------------------------------------------------------------------
void eKeyboard::Init()
{
	Reset();
}
//=============================================================================
//	eKeyboard::Reset
//-----------------------------------------------------------------------------
void eKeyboard::Reset()
{
	memset(kbd, 0xff, sizeof(kbd));
}
//=============================================================================
//	eKeyboard::IoRead
//-----------------------------------------------------------------------------
bool eKeyboard::IoRead(word port) const
{
	return !(port&1);
}
//=============================================================================
//	eKeyboard::IoRead
//-----------------------------------------------------------------------------
void eKeyboard::IoRead(word port, byte* v, int tact)
{
	*v = Read(port >> 8);
}
//=============================================================================
//	eKeyboard::Read
//-----------------------------------------------------------------------------
byte eKeyboard::Read(byte scan) const
{
	byte res = 0xbf;
	for(int i = 0; i < 8; i++)
	{
		if(!(scan & (1<<i)))
			res &= kbd[i];
	}
	return res;
}
//=============================================================================
//	eKey
//-----------------------------------------------------------------------------
struct eKey
{
	char key;
	byte mask;
	byte offset;
};
enum { KEYS_COUNT = 40 };
static const eKey keys[KEYS_COUNT] =
{
	{'c', ~0x01, 0},
	{'Z', ~0x02, 0},
	{'X', ~0x04, 0},
	{'C', ~0x08, 0},
	{'V', ~0x10, 0},

	{'A', ~0x01, 1},
	{'S', ~0x02, 1},
	{'D', ~0x04, 1},
	{'F', ~0x08, 1},
	{'G', ~0x10, 1},

	{'Q', ~0x01, 2},
	{'W', ~0x02, 2},
	{'E', ~0x04, 2},
	{'R', ~0x08, 2},
	{'T', ~0x10, 2},

	{'1', ~0x01, 3},
	{'2', ~0x02, 3},
	{'3', ~0x04, 3},
	{'4', ~0x08, 3},
	{'5', ~0x10, 3},

	{'0', ~0x01, 4},
	{'9', ~0x02, 4},
	{'8', ~0x04, 4},
	{'7', ~0x08, 4},
	{'6', ~0x10, 4},

	{'P', ~0x01, 5},
	{'O', ~0x02, 5},
	{'I', ~0x04, 5},
	{'U', ~0x08, 5},
	{'Y', ~0x10, 5},

	{'e', ~0x01, 6},
	{'L', ~0x02, 6},
	{'K', ~0x04, 6},
	{'J', ~0x08, 6},
	{'H', ~0x10, 6},

	{' ', ~0x01, 7},
	{'s', ~0x02, 7},
	{'M', ~0x04, 7},
	{'N', ~0x08, 7},
	{'B', ~0x10, 7},
};
//=============================================================================
//	eKeyboard::OnKey
//-----------------------------------------------------------------------------
void eKeyboard::OnKey(char _key, bool _down, bool _shift, bool _ctrl, bool _alt)
{
	KeyState(_key, _down);
	KeyState('c', _shift);	// caps shift
	KeyState('s', _alt);	// symbol shift
}
//=============================================================================
//	eKeyboard::KeyState
//-----------------------------------------------------------------------------
void eKeyboard::KeyState(char _key, bool _down)
{
	for(int i = 0; i < KEYS_COUNT; ++i)
	{
		const eKey& k = keys[i];
		if(_key == k.key)
		{
			if(_down)
				kbd[k.offset] &= k.mask;
			else
				kbd[k.offset] |= ~k.mask;
			break;
		}
	}
}
