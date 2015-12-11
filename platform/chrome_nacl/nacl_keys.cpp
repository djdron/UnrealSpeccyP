/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _CHROME_NACL

#include "../platform.h"

namespace xPlatform
{

void TranslateKey(int& key, dword& flags)
{
	enum
	{
		K_SHIFT = 16, K_CTRL = 17, K_ALT = 18,
		K_TAB = 9, K_ENTER = 13, K_BACKSPACE = 8,
		K_LEFT = 37, K_UP = 38, K_RIGHT = 39, K_DOWN = 40,
		K_QUOTE = 222, K_APOSTROPHE = 192, K_BACKSLASH = 220,
		K_DOT = 190, K_COMMA = 188, K_SEMICOLON = 186, K_SLASH = 191,
		K_MINUS = 189, K_EQUAL = 187,
	};
	switch(key)
	{
	case K_SHIFT:		key = 'c';	break;
	case K_ALT:			key = 's';	break;
	case K_ENTER:		key = 'e';	break;
	case K_TAB:
		key = '\0';
		flags |= KF_ALT;
		flags |= KF_SHIFT;
		break;
	case K_BACKSPACE:
		key = '0';
		flags |= KF_SHIFT;
		break;
	case K_LEFT:		key = 'l';	break;
	case K_RIGHT:		key = 'r';	break;
	case K_UP:			key = 'u';	break;
	case K_DOWN:		key = 'd';	break;
	case K_CTRL:		key = 'f';	flags &= ~KF_CTRL; break;
	case K_APOSTROPHE:	key = 'm';	break;
	case K_BACKSLASH:	key = 'k';	break;
	case K_QUOTE:
		if(flags&KF_SHIFT)
		{
			key = 'P';
			flags &= ~KF_SHIFT;
		}
		else
			key = '7';
		flags |= KF_ALT;
		break;
	case K_COMMA:
		if(flags&KF_SHIFT)
		{
			key = 'R';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'N';
		flags |= KF_ALT;
		break;
	case K_DOT:
		if(flags&KF_SHIFT)
		{
			key = 'T';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'M';
		flags |= KF_ALT;
		break;
	case K_SEMICOLON:
		if(flags&KF_SHIFT)
		{
			key = 'Z';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'O';
		flags |= KF_ALT;
		break;
	case K_SLASH:
		if(flags&KF_SHIFT)
		{
			key = 'C';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'V';
		flags |= KF_ALT;
		break;
	case K_MINUS:
		if(flags&KF_SHIFT)
		{
			key = '0';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'J';
		flags |= KF_ALT;
		break;
	case K_EQUAL:
		if(flags&KF_SHIFT)
		{
			key = 'K';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'L';
		flags |= KF_ALT;
		break;
	}
	if(key > 255 || key < 32)
		key = 0;
}

}
//namespace xPlatform

#endif//_CHROME_NACL
