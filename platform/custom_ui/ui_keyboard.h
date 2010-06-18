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

#ifndef	__UI_KEYBOARD_H__
#define	__UI_KEYBOARD_H__

#include "../../ui/ui_dialog.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eKeyboard : public eDialog
{
	typedef eDialog eInherited;
public:
	eKeyboard() : key(0), pressed(false), caps(false), symbol(false), flags(0) {}
	virtual void Init();
	byte Key() const { return key; }
	bool Pressed() const { return pressed; }
	bool Caps() const { return caps; }
	bool Symbol() const { return symbol; }
	virtual bool OnKey(char key, dword flags);
	enum eId { ID_CAPS = 0, ID_SYMBOL };
protected:
	virtual void OnNotify(byte n, byte from);
	byte AllocateId(const char* key) const;
protected:
	byte key;
	bool pressed;
	bool caps;
	bool symbol;
	dword flags;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_KEYBOARD_H__
