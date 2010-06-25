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

#ifndef	__UI_FILE_OPEN_H__
#define	__UI_FILE_OPEN_H__

#include "../../ui/ui_dialog.h"
#include "../io.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eList;

class eFileOpenDialog : public eDialog
{
	enum { MAX_ITEMS = 2000 };
	typedef eDialog eInherited;
public:
	eFileOpenDialog(const char* path);
	virtual void Init();
	const char* Selected() { return selected; }
protected:
	void OnNotify(byte n, byte from);
	void OnChangePath();
protected:
	char path[xIo::MAX_PATH_LEN];
	eList* list;
	bool folders[MAX_ITEMS];
	const char* selected;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_FILE_OPEN_H__
