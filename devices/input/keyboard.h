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

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "../device.h"

#pragma once

class eKeyboard : public eDevice
{
public:
	virtual void Init();
	virtual void Reset();
	virtual bool IoRead(word port) const;
	virtual void IoRead(word port, byte* v, int tact);
	void OnKey(char key, bool down, bool shift, bool ctrl, bool alt);

	static eDeviceId Id() { return D_KEYBOARD; }
	virtual dword IoNeed() const { return ION_READ; }
protected:
	void KeyState(char key, bool down);
	byte Read(byte scan) const;
	byte kbd[8];
};

#endif//__KEYBOARD_H__
