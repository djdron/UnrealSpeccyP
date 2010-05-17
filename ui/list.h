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

#ifndef	__UI_LIST_H__
#define	__UI_LIST_H__

#include "control.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eList : public eControl
{
	enum { MAX_ITEMS = 2000 };
	enum { CURSOR_COLOR = 0x08b06000 };
	enum eNotify { N_SELECTED };
public:
	eList() : size(0), last_selected(0), selected(0), page_begin(0), page_size(0) { *items = NULL; }
	virtual ~eList() { Clear(); }
	void Clear()
	{
		changed = true;
		for(int i = 0; items[i]; ++i)
		{
			delete[] items[i];
		}
		*items = NULL;
		size = last_selected = selected = page_begin = page_size = 0;
	}
	void Insert(const char* item);
	const char* Selected() const { return size ? items[selected] : NULL; }
	int Selector() const { return size ? selected : -1; }
	virtual void Update();
	virtual void OnKey(char key, dword flags);
protected:
	const char* items[MAX_ITEMS + 1];
	int size;
	int last_selected;
	int selected;
	int page_begin;
	int page_size;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_LIST_H__
