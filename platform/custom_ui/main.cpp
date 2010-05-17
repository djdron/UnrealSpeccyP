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
#include "../platform.h"
#include "../io.h"
#include "main.h"
#include "menu.h"
#include "keyboard.h"
#include "file_open.h"

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eMainDialog::eMainDialog
//-----------------------------------------------------------------------------
eMainDialog::eMainDialog() : clear(false), open_file(false)
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
	if(open_file)
	{
		open_file = false;
		Clear();
		eDialog* d = new eFileOpenDialog(path);
		d->Id(D_FILE_OPEN);
		Insert(d);
	}
}
//=============================================================================
//	eMainDialog::SetupMenu
//-----------------------------------------------------------------------------
void eMainDialog::SetupMenu()
{
	using namespace xPlatform;
	eMenuDialog* d = (eMenuDialog*)*childs;
	int v = Handler()->TapeInserted() ? Handler()->TapeStarted() ? 0 : 1 : 2;
	d->ItemState(eMenuDialog::I_TAPE, v);
	d->ItemState(eMenuDialog::I_FAST_TAPE, !Handler()->FullSpeed());
	d->ItemState(eMenuDialog::I_JOYSTICK, Handler()->Joystick());
	d->ItemState(eMenuDialog::I_SOUND, Handler()->Sound());
	d->ItemState(eMenuDialog::I_VOLUME, Handler()->Volume());
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
			eMenuDialog* d = new eMenuDialog;
			Insert(d);
			d->Id(D_MENU);
			SetupMenu();
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
		eMenuDialog* d = (eMenuDialog*)*childs;
		switch(n)
		{
		case eMenuDialog::I_OPEN:
			open_file = true;
			break;
		case eMenuDialog::I_JOYSTICK:
			Handler()->OnAction(A_JOYSTICK_NEXT);
			d->ItemState(n, Handler()->Joystick());
			break;
		case eMenuDialog::I_TAPE:
			{
				Handler()->OnAction(A_TAPE_TOGGLE);
				int v = Handler()->TapeInserted() ? Handler()->TapeStarted() ? 0 : 1 : 2;
				d->ItemState(n, v);
			}
			break;
		case eMenuDialog::I_FAST_TAPE:
			Handler()->OnAction(A_TAPE_FAST_TOGGLE);
			d->ItemState(eMenuDialog::I_FAST_TAPE, !Handler()->FullSpeed());
			break;
		case eMenuDialog::I_SOUND:
			Handler()->OnAction(A_SOUND_NEXT);
			d->ItemState(n, Handler()->Sound());
			break;
		case eMenuDialog::I_VOLUME:
			Handler()->OnAction(A_VOLUME_NEXT);
			d->ItemState(n, Handler()->Volume());
			break;
		case eMenuDialog::I_RESET:
			Handler()->OnAction(A_RESET);
			break;
		case eMenuDialog::I_QUIT:
			Handler()->OnAction(A_QUIT);
			break;
		}
		break;
	}
}

}
//namespace xUi

#endif//USE_UI
