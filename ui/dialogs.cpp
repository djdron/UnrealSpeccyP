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

#include "../std.h"
#include "dialogs.h"

#ifdef _WINDOWS
#include <io.h>
#endif//_WINDOWS

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eFileOpenDialog::Init
//-----------------------------------------------------------------------------
void eFileOpenDialog::Init()
{
	background = BACKGROUND_COLOR;
	eRect r(8, 8, 120, 180);
	Bound() = r;
	list = new eList;
	list->Bound() = eRect(6, 6, r.Width() - 6, r.Height() - 6);
	Insert(list);
	OnChangePath();
}
//=============================================================================
//	eFileOpenDialog::OnChangePath
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnChangePath()
{
#ifndef _DINGOO
	list->Clear();
#ifdef _LINUX
	list->Insert("FILE 1");
	list->Insert("FILE 2");
	list->Insert("FILE 3");
#endif//_LINUX
#ifdef _WINDOWS
	_finddata_t fd;
	dword handle = _findfirst(path, &fd);
	dword res = handle;
	memset(folders, 0, sizeof(folders));
	int i = 0;
	while(res != -1)
	{
		if(strcmp(fd.name, "."))
		{
			folders[i++] = fd.attrib&0x10;
			list->Insert(fd.name);
		}
		res = _findnext(handle, &fd);
	}
	_findclose(handle);
#endif//_WINDOWS
#else
	eFindData fd;
	int res = fsys_findfirst(path, -1, &fd);
	list->Clear();
	memset(folders, 0, sizeof(folders));
	int i = 0;
	while(!res)
	{
		if(strcmp(fd.name, "."))
		{
			folders[i++] = fd.attrib&0x10;
			list->Insert(fd.name);
		}
		res = fsys_findnext(&fd);
	}
	fsys_findclose(&fd);
#endif
}
static void GetUpLevel(char* path, int level = 1)
{
	for(int i = strlen(path); --i >= 0; )
	{
		if(((path[i] == '\\') || (path[i] == '/')) && !--level)
		{
			path[i + 1] = '\0';
			return;
		}
	}
}
//=============================================================================
//	eFileOpenDialog::OnKey
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnKey(char key)
{
	if(key == 'e' && list->Selected())
	{
		if(folders[list->Selector()])
		{
			GetUpLevel(path);
			strcat(path, list->Selected());
			strcat(path, "\\*.*");
			OnChangePath();
			return;
		}
		GetUpLevel(path);
		strcat(path, list->Selected());
		selected = path;
		return;
	}
	eInherited::OnKey(key);
}

static const char* zx_keys[] =
{
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
	"A", "S", "D", "F", "G", "H", "J", "K", "L", "En",
	"Cs", "Z", "X", "C", "V", "B", "N", "M", "Ss", "Sp"
};

//=============================================================================
//	eKeysDialog::AllocateId
//-----------------------------------------------------------------------------
dword eKeysDialog::AllocateId(const char* key) const
{
	if(strlen(key) == 1)	return *key;
	if(!strcmp(key, "Cs"))	return ID_CAPS;
	if(!strcmp(key, "Ss"))	return ID_SYMBOL;
	if(!strcmp(key, "En"))	return ID_ENTER;
	if(!strcmp(key, "Sp"))	return ID_SPACE;
	return -1;
}
//=============================================================================
//	eKeysDialog::Init
//-----------------------------------------------------------------------------
void eKeysDialog::Init()
{
	background = BACKGROUND_COLOR;
	eRect r_dlg(ePoint(200, 80));
	r_dlg.Move(ePoint(112, 8));
	Bound() = r_dlg;
	eRect r_item(ePoint(13, FontSize().y + 2));
	ePoint margin(8, 8);
	ePoint delta;
	delta.x = (r_dlg.Width() - r_item.Width() - margin.x * 2) / 9;
	delta.y = (r_dlg.Height() - r_item.Height() - margin.y * 2) / 3;
	r_item.Move(margin);
	for(int col = 0; col < 4; ++col)
	{
		eRect r = r_item;
		for(int row = 0; row < 10; ++row)
		{
			eButton* b = new eButton;
			b->Bound() = r;
			const char* s = zx_keys[col * 10 + row];
			b->Id(AllocateId(s));
			b->Text(s);
			Insert(b);
			r.Move(ePoint(delta.x, 0));
		}
		r_item.Move(ePoint(0, delta.y));
	}
}


//=============================================================================
//	eManager::Update
//-----------------------------------------------------------------------------
void eManager::Update()
{
	if(key)
	{
		if(keypress_timer > KEY_REPEAT_DELAY)
		{
			SAFE_CALL(fo_dialog)->OnKey(key);
			SAFE_CALL(keys_dialog)->OnKey(key);
		}
		++keypress_timer;
	}
	SAFE_CALL(fo_dialog)->Update();
	SAFE_CALL(keys_dialog)->Update();
	if(fo_dialog && fo_dialog->Selected())
	{
		xPlatform::Handler()->OnOpenFile(fo_dialog->Selected());
		SAFE_DELETE(fo_dialog);
	}
	if(keys_dialog)
	{
		byte key = keys_dialog->Key();
		dword flags = key ? xPlatform::KF_DOWN : 0;
		xPlatform::Handler()->OnKey(key, flags);
	}
}
//=============================================================================
//	eManager::OnKey
//-----------------------------------------------------------------------------
void eManager::OnKey(char _key, bool pressed)
{
	if((pressed && (_key == key)) || (!pressed && (_key != key)))
		return;
	key = pressed ? _key : '\0';
	if(!key)
		keypress_timer = 0;
	SAFE_CALL(fo_dialog)->OnKey(key);
	SAFE_CALL(keys_dialog)->OnKey(key);
	if(key == '`' || key == '~')
	{
		if(!fo_dialog && !keys_dialog)
		{
			if(key == '`')
			{
				fo_dialog = new eFileOpenDialog(path);
			}
			else
			{
				keys_dialog = new eKeysDialog;
			}
			SAFE_CALL(fo_dialog)->Init();
			SAFE_CALL(keys_dialog)->Init();
		}
		else
		{
			SAFE_DELETE(fo_dialog);
			SAFE_DELETE(keys_dialog);
		}
	}
}

}
//namespace xUi

#endif//USE_UI
