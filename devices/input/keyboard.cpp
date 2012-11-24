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
	{'c', byte(~0x01), 0},
	{'Z', byte(~0x02), 0},
	{'X', byte(~0x04), 0},
	{'C', byte(~0x08), 0},
	{'V', byte(~0x10), 0},

	{'A', byte(~0x01), 1},
	{'S', byte(~0x02), 1},
	{'D', byte(~0x04), 1},
	{'F', byte(~0x08), 1},
	{'G', byte(~0x10), 1},

	{'Q', byte(~0x01), 2},
	{'W', byte(~0x02), 2},
	{'E', byte(~0x04), 2},
	{'R', byte(~0x08), 2},
	{'T', byte(~0x10), 2},

	{'1', byte(~0x01), 3},
	{'2', byte(~0x02), 3},
	{'3', byte(~0x04), 3},
	{'4', byte(~0x08), 3},
	{'5', byte(~0x10), 3},

	{'0', byte(~0x01), 4},
	{'9', byte(~0x02), 4},
	{'8', byte(~0x04), 4},
	{'7', byte(~0x08), 4},
	{'6', byte(~0x10), 4},

	{'P', byte(~0x01), 5},
	{'O', byte(~0x02), 5},
	{'I', byte(~0x04), 5},
	{'U', byte(~0x08), 5},
	{'Y', byte(~0x10), 5},

	{'e', byte(~0x01), 6},
	{'L', byte(~0x02), 6},
	{'K', byte(~0x04), 6},
	{'J', byte(~0x08), 6},
	{'H', byte(~0x10), 6},

	{' ', byte(~0x01), 7},
	{'s', byte(~0x02), 7},
	{'M', byte(~0x04), 7},
	{'N', byte(~0x08), 7},
	{'B', byte(~0x10), 7},
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
