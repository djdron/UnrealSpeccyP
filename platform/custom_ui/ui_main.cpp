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

#include "../platform.h"
#include "../io.h"
#include "ui_main.h"
#include "ui_menu.h"
#include "ui_keyboard.h"
#include "ui_file_open.h"
#include "../../tools/option.h"

#ifdef USE_UI

namespace xUi
{

static struct eOptionOpenFile : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "open file"; }
	virtual const char** Values() const
	{
		static const char* values[] = { ">", ">", NULL };
		return values;
	}
	virtual void Change(bool next = true) { ValueBool(true); }
} op_open_file;

//=============================================================================
//	eMainDialog::eMainDialog
//-----------------------------------------------------------------------------
eMainDialog::eMainDialog() : clear(false)
{
	strcpy(path, xIo::ResourcePath("\\*.*"));
}
//=============================================================================
//	eMainDialog::Update
//-----------------------------------------------------------------------------
void eMainDialog::Update()
{
	eInherited::Update();
	if(clear)
	{
		clear = false;
		Clear();
	}
	if(op_open_file.ValueBool())
	{
		op_open_file.ValueBool(false);
		Clear();
		eDialog* d = new eFileOpenDialog(path);
		d->Id(D_FILE_OPEN);
		Insert(d);
	}
}
//=============================================================================
//	eMainDialog::OnKey
//-----------------------------------------------------------------------------
void eMainDialog::OnKey(char key, dword flags)
{
	eInherited::OnKey(key, flags);
	switch(key)
	{
	case '\\':
		if(!Focused())
		{
			eKeyboard* d = new eKeyboard;
			d->Id(D_KEYS);
			Insert(d);
			return;
		}
		Clear();
		break;
	case '`':
		if(!Focused() || (*childs)->Id() == D_FILE_OPEN)
		{
			Clear();
			using namespace xPlatform;
			eMenu* d = new eMenu;
			d->Id(D_MENU);
			Insert(d);
			return;
		}
		Clear();
		break;
	}
}
//=============================================================================
//	eMainDialog::OnNotify
//-----------------------------------------------------------------------------
void eMainDialog::OnNotify(byte n, byte from)
{
	using namespace xPlatform;
	switch(from)
	{
	case D_FILE_OPEN:
		{
			eFileOpenDialog* d = (eFileOpenDialog*)*childs;
			Handler()->OnOpenFile(d->Selected());
			strcpy(path, d->Selected());
			GetUpLevel(path);
			strcat(path, "*.*");
			clear = true;
		}
		break;
	case D_KEYS:
		{
			eKeyboard* d = (eKeyboard*)*childs;
			byte key = d->Key();
			dword flags = d->Pressed() ? KF_DOWN : 0;
			flags |= d->Caps() ? KF_SHIFT : 0;
			flags |= d->Symbol() ? KF_ALT : 0;
			flags |= KF_UI_SENDER;
			Handler()->OnKey(key, flags);
		}
		break;
	case D_MENU:
		eMenu* d = (eMenu*)*childs;
		d->ChangeItem(n);
		break;
	}
}

}
//namespace xUi

#endif//USE_UI
