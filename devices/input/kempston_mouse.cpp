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
#include "kempston_mouse.h"

void eKempstonMouse::Init() { Reset(); }
//=============================================================================
//	eKempstonMouse::Reset
//-----------------------------------------------------------------------------
void eKempstonMouse::Reset()
{
    x = 31; // assign random different coords (some programs test this)
    y = 85;
    buttons = 0xFF;
}
//=============================================================================
//	eKempstonMouse::IoRead
//-----------------------------------------------------------------------------
bool eKempstonMouse::IoRead(word port) const
{
	if(port&0x20)
		return false;
	port |= 0xfa00; // A13,A15 not used in decoding
	if(port != 0xfadf && port != 0xfbdf && port != 0xffdf)
		return false;
	return true;
}
//=============================================================================
//	eKempstonMouse::IoRead
//-----------------------------------------------------------------------------
void eKempstonMouse::IoRead(word port, byte* v, int tact)
{
    port |= 0xfa00; // A13,A15 not used in decoding
    switch(port)
    {
    case 0xfbdf: *v = x;		break;
    case 0xffdf: *v = y;		break;
    case 0xfadf: *v = buttons;	break;
    }
}
//=============================================================================
//	eKempstonMouse::OnMouseMove
//-----------------------------------------------------------------------------
void eKempstonMouse::OnMouseMove(byte _dx, byte _dy)
{
	x += _dx;
	y += _dy;
}
//=============================================================================
//	eKempstonMouse::OnMouseButton
//-----------------------------------------------------------------------------
void eKempstonMouse::OnMouseButton(byte index, bool state)
{
	byte v = byte(0x01) << index;
	if(state)
		buttons &= ~v;
	else
		buttons |= v;
}
