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

#ifndef	__UI_BUTTON_H__
#define	__UI_BUTTON_H__

#include "ui_control.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eButton : public eControl
{
	enum { MAX_TEXT_SIZE = 64 };
	enum { PUSH_COLOR = 0x080000b0, PUSH_FOCUS_COLOR = 0x08800080 };
	typedef eControl eInherited;
public:
	eButton() : pushed(false), triggered(false), last_pushed(false), last_key(0), highlight(true) { *text = '\0'; }
	void Highlight(bool on) { highlight = on; }
	void Text(const char* s);
	void Push(bool b) { pushed = b; triggered = false; }
	virtual void Update();
	virtual bool OnKey(char key, dword flags);
	enum eNotify { N_PUSH, N_POP };
protected:
	char text[MAX_TEXT_SIZE + 1];
	bool pushed;
	bool triggered;
	bool last_pushed;
	char last_key;
	bool highlight;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_BUTTON_H__
