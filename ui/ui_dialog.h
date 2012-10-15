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

#ifndef	__UI_DIALOG_H__
#define	__UI_DIALOG_H__

#include "ui_control.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eDialog : public eControl
{
	enum { MAX_CHILDS = 64 };
public:
	eDialog() : focused(NULL) { *childs = NULL; }
	virtual ~eDialog()
	{
		Clear();
		DrawRect(bound, COLOR_NONE);
	}
	void Insert(eControl* child);
	void Clear()
	{
		for(int i = 0; childs[i]; ++i)
		{
			delete childs[i];
		}
		*childs = focused = NULL;
	}
	eControl** Childs() { return childs; }
	eControl* const* Childs() const { return childs; }
	virtual void Update();
	virtual bool OnKey(char key, dword flags);
protected:
	void ChooseFocus(char key);

protected:
	eControl* childs[MAX_CHILDS + 1];
	eControl* focused;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_DIALOG_H__
