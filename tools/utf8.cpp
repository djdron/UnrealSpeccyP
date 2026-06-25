/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "utf8.h"

namespace xUtf8
{

//=============================================================================
//	ProcessChar
//-----------------------------------------------------------------------------
static void ProcessChar(dword* c, const char** s, int size)
{
	enum { UTF_8_MASK = 0xc0/*11000000*/, UTF_8_SIGN = 0x80/*10000000*/, UTF_8_SHIFT = 6 };
	while(--size >= 0)
	{
		if((**s&UTF_8_MASK) == UTF_8_SIGN)
		{
			*c <<= UTF_8_SHIFT;
			*c |= (byte)(**s&~UTF_8_MASK);
			++*s;
		}
		else
			break;
	}
}
//=============================================================================
//	Next
//-----------------------------------------------------------------------------
static const byte utf8_mask[] =
{
	0xe0/*11100000*/,
	0xf0/*11110000*/,
	0xf8/*11111000*/
};
dword Next(const char** s)
{
	dword c = (byte)**s;
	if(!c)
		return c;
	++*s;
	for(int i = 0; i < 3; ++i)
	{
		byte mask = utf8_mask[i];
		if((c&mask) == byte(mask << 1))
		{
			c &= ~mask;
			ProcessChar(&c, s, i + 1);
			break;
		}
	}
	return c;
}

}
//namespace xUtf8
