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

#ifndef	__UI_DESKTOP_H__
#define	__UI_DESKTOP_H__

#include "ui_dialog.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eDesktop : public eDialog
{
	enum { KEY_REPEAT_DELAY = 10 };
	typedef eDialog eInherited;
public:
	eDesktop() : key(0), key_flags(0), keypress_timer(0)
	{
		_CreateFont(6, 6, "res/font/spxtrm4f.fnt");
	}
	dword* VideoData() const { return Focused() ? Screen() : NULL; }
	bool Focused() const
	{
		eDialog* d = (eDialog*)Childs()[0];
		if(d)
			return d->Childs()[0] != NULL;
		return false;
	}
	virtual void Update();
	virtual void OnKey(char key, dword flags);

protected:
	char key;
	dword key_flags;
	int keypress_timer;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_DESKTOP_H__
