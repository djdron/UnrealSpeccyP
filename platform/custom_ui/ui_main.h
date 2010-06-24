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

#ifndef	__UI_MAIN_H__
#define	__UI_MAIN_H__

#include "../../ui/ui_dialog.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eMainDialog : public eDialog
{
	typedef eDialog eInherited;
public:
	eMainDialog();
	virtual void Update();
	virtual bool OnKey(char key, dword flags);
protected:
	virtual void OnNotify(byte n, byte from);
	enum eDialogId { D_FILE_OPEN, D_KEYS, D_MENU, D_PROFILER };
	bool Focused() const { return childs[0] != NULL; }
protected:
	bool clear;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_MAIN_H__
