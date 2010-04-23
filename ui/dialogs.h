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
#include "../platform/platform.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

//*****************************************************************************
//	eFileOpenDialog
//-----------------------------------------------------------------------------
class eFileOpenDialog : public eDialog
{
	typedef eDialog eInherited;
public:
	eFileOpenDialog(const char* _path) : list(NULL), selected(NULL)
	{
		strcpy(path, _path);
		memset(folders, 0, sizeof(folders));
	}
	virtual void Init();
	const char* Selected() { return selected; }
	virtual void OnKey(char key);
protected:
	void OnChangePath();
	enum { BACKGROUND_COLOR = 0x01202020 };
protected:
	char path[256];
	eList* list;
	bool folders[256];
	const char* selected;
};

//*****************************************************************************
//	eKeysDialog
//-----------------------------------------------------------------------------
class eKeysDialog : public eDialog
{
public:
	eKeysDialog() : key(0) {}
	virtual void Init();
	byte Key() const { return key; }
	enum eId { ID_CAPS = 0, ID_SYMBOL, ID_ENTER, ID_SPACE };
protected:
	virtual void OnNotify(dword id) { key = id; }
	dword AllocateId(const char* key) const;
	enum { BACKGROUND_COLOR = 0x01202020 };
protected:
	byte key;
};


//*****************************************************************************
//	eManager
//-----------------------------------------------------------------------------
class eManager
{
public:
	eManager(const char* _path) : fo_dialog(NULL), keys_dialog(NULL), key('\0'), keypress_timer(0)
	{
		strcpy(path, _path);
	}
	~eManager()
	{
		SAFE_DELETE(fo_dialog);
		SAFE_DELETE(keys_dialog);
	}
	void Init()
	{
		CreateFont(6, 6, "res/font/spxtrm4f.fnt");
	}
	dword* VideoData() const { return (fo_dialog || keys_dialog) ? Screen() : NULL; }
	bool Focused() const { return fo_dialog || keys_dialog; }
	void Update();
	void OnKey(char _key, bool pressed);
	enum { KEY_REPEAT_DELAY = 10 };
protected:
	char path[256];
	eFileOpenDialog* fo_dialog;
	eKeysDialog* keys_dialog;
	char key;
	int keypress_timer;
};

}
//namespace xUi

#endif//USE_UI

#endif//__DIALOGS_H__
