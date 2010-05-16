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

#ifndef	__DIALOGS_H__
#define	__DIALOGS_H__

#include "controls.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

//*****************************************************************************
//	eFileOpenDialog
//-----------------------------------------------------------------------------
class eFileOpenDialog : public eDialog
{
	enum { MAX_ITEMS = 2000 };
	typedef eDialog eInherited;
public:
	eFileOpenDialog(const char* _path) : list(NULL), selected(NULL)
	{
		strcpy(path, _path);
		memset(folders, 0, sizeof(folders));
	}
	virtual void Init();
	const char* Selected() { return selected; }
protected:
	void OnNotify(byte n, byte from);
	void OnChangePath();
protected:
	char path[256];
	eList* list;
	bool folders[MAX_ITEMS];
	const char* selected;
};

//*****************************************************************************
//	eKeysDialog
//-----------------------------------------------------------------------------
class eKeysDialog : public eDialog
{
	typedef eDialog eInherited;
public:
	eKeysDialog() : key(0), pressed(false), caps(false), symbol(false), flags(0) {}
	virtual void Init();
	byte Key() const { return key; }
	bool Pressed() const { return pressed; }
	bool Caps() const { return caps; }
	bool Symbol() const { return symbol; }
	virtual void OnKey(char key, dword flags);
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

//*****************************************************************************
//	eMenuDialog
//-----------------------------------------------------------------------------
class eMenuDialog : public eDialog
{
	typedef eDialog eInherited;
public:
	eMenuDialog() {}
	virtual void Init();
	void ItemState(int idx, int v)
	{
		char s[80];
		GetItemText(idx, v, s);
		((eButton*)childs[idx])->Text(s);
	}
	enum eItemId { I_OPEN, I_JOYSTICK, I_TAPE, I_FAST_TAPE, I_SOUND, I_VOLUME, I_RESET, I_QUIT, I_COUNT };
protected:
	virtual void OnNotify(byte n, byte from);
	void GetItemText(int idx, int state, char* dst) const;
};

//*****************************************************************************
//	eMainDialog
//-----------------------------------------------------------------------------
class eMainDialog : public eDialog
{
	typedef eDialog eInherited;
public:
	eMainDialog();
	bool Focused() const { return *childs; }
	virtual void Update();
	virtual void OnKey(char key, dword flags);
protected:
	virtual void OnNotify(byte n, byte from);
	void SetupMenu();
	enum eDialogId { D_FILE_OPEN, D_KEYS, D_MENU };
protected:
	bool clear;
	bool open_file;
	char path[256];
};


//*****************************************************************************
//	eManager
//-----------------------------------------------------------------------------
class eManager : public eDialog
{
	enum { KEY_REPEAT_DELAY = 10 };
	typedef eDialog eInherited;
public:
	eManager() : key(0), key_flags(0), keypress_timer(0)
	{
		CreateFont(6, 6, "res/font/spxtrm4f.fnt");
		Insert(new eMainDialog);
	}
	dword* VideoData() const { return Focused() ? Screen() : NULL; }
	bool Focused() const { return ((eMainDialog*)*childs)->Focused(); }
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

#endif//__DIALOGS_H__
