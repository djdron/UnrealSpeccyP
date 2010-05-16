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
#include "../platform/platform.h"
#include "../platform/io.h"

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
	ePoint margin(6, 6);
	Bound() = r;
	list = new eList;
	list->Bound() = eRect(margin.x, margin.y, r.Width() - margin.x, r.Height() - margin.y);
	Insert(list);
	OnChangePath();
}
//=============================================================================
//	eFileOpenDialog::OnChangePath
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnChangePath()
{
	list->Clear();
	memset(folders, 0, sizeof(folders));
	int i = 0;

#ifdef _LINUX
	list->Insert("FILE 1");
	list->Insert("FILE 2");
	list->Insert("FILE 3");
#endif//_LINUX

#ifdef _WINDOWS
	_finddata_t fd;
	dword handle = _findfirst(path, &fd);
	dword res = handle;
	while(res != -1 && i < MAX_ITEMS)
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

#ifdef _DINGOO
	eFindData fd;
	int res = fsys_findfirst(path, -1, &fd);
	int levels = 0;
	for(const char* src = path; *src; ++src)
	{
		if(*src == '\\' || *src == '/')
			++levels;
	}
	if(levels > 1) //isn't root
	{
		list->Insert("..");
		folders[i++] = true;
	}
	while(!res && i < MAX_ITEMS)
	{
		folders[i++] = fd.attrib&0x10;
		list->Insert(fd.name);
		res = fsys_findnext(&fd);
	}
	fsys_findclose(&fd);
#endif//_DINGOO
}

static void GetUpLevel(char* path, int level = 1)
{
	for(int i = strlen(path); --i >= 0; )
	{
		if(((path[i] == '\\') || (path[i] == '/')))
		{
			while(--i >= 0 && ((path[i] == '\\') || (path[i] == '/')));
			++i;
			if(!--level)
			{
				path[i + 1] = '\0';
				return;
			}
		}
	}
}

//=============================================================================
//	eFileOpenDialog::OnNotify
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnNotify(byte n, byte from)
{
	if(list->Selected())
	{
		if(folders[list->Selector()])
		{
			if(!strcmp(list->Selected(), ".."))
			{
				GetUpLevel(path, 2);
				strcat(path, "*.*");
			}
			else
			{
				GetUpLevel(path);
				strcat(path, list->Selected());
				strcat(path, "\\*.*");
			}
			OnChangePath();
			return;
		}
		GetUpLevel(path);
		strcat(path, list->Selected());
		selected = path;
		eInherited::OnNotify(n, id);
	}
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
byte eKeysDialog::AllocateId(const char* key) const
{
	if(strlen(key) == 1)	return *key;
	if(!strcmp(key, "Cs"))	return 'c';
	if(!strcmp(key, "Ss"))	return 's';
	if(!strcmp(key, "En"))	return 'e';
	if(!strcmp(key, "Sp"))	return ' ';
	return -1;
}
//=============================================================================
//	eKeysDialog::Init
//-----------------------------------------------------------------------------
void eKeysDialog::Init()
{
	background = BACKGROUND_COLOR;
	eRect r_dlg(ePoint(169, 70));
	r_dlg.Move(ePoint(143, 8));
	Bound() = r_dlg;
	eRect r_item(ePoint(13, FontSize().y + 2));
	ePoint margin(6, 6);
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
//	eKeysDialog::OnKey
//-----------------------------------------------------------------------------
void eKeysDialog::OnKey(char key, dword _flags)
{
	using namespace xPlatform;
	if((_flags&KF_SHIFT) != (flags&KF_SHIFT))
		((eButton*)childs[30])->Push(_flags&KF_SHIFT);
	if((_flags&KF_ALT) != (flags&KF_ALT))
		((eButton*)childs[38])->Push(_flags&KF_ALT);
	flags = _flags;
	eInherited::OnKey(key, flags);
}
//=============================================================================
//	eKeysDialog::OnNotify
//-----------------------------------------------------------------------------
void eKeysDialog::OnNotify(byte n, byte from)
{
	bool pushed = n == eButton::N_PUSH;
	switch(from)
	{
	case 'c': caps = pushed;	break;
	case 's': symbol = pushed;	break;
	default:
		key = from;
		pressed = pushed;
	}
	eInherited::OnNotify(n, id);
}

static const char* menu_open[] = { ">" };
static const char* menu_joystick[] = { "kempston", "cursor", "qaop", "sinclair2" };
static const char* menu_tape[] = { "start", "stop", "n/a" };
static const char* menu_tape_fast[] = { "on", "off" };
static const char* menu_sound[] = { "beeper", "ay", "tape" };
static const char* menu_volume[] = { "mute", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%" };

static const char* menu_items[] = { "open image", "joystick", "tape", "fast tape", "sound", "volume", "reset", "quit" };
static const char** menu_states[] = { menu_open, menu_joystick, menu_tape, menu_tape_fast, menu_sound, menu_volume, NULL, NULL };

//=============================================================================
//	eMenuDialog::GetItemText
//-----------------------------------------------------------------------------
void eMenuDialog::GetItemText(int idx, int state, char* dst) const
{
	strcpy(dst, menu_items[idx]);
	int offs = strlen(dst);
	int spc_count = childs[idx]->Bound().Width() / FontSize().x - offs;
	const char** states_text = menu_states[idx];
	spc_count -= states_text ? strlen(states_text[state]) : 0;
	for(int i = 0; i < spc_count; ++i)
	{
		dst[offs++] = ' ';
	}
	dst[offs] = '\0';
	if(states_text)
	{
		strcat(dst, states_text[state]);
	}
}
//=============================================================================
//	eMenuDialog::Init
//-----------------------------------------------------------------------------
void eMenuDialog::Init()
{
	background = BACKGROUND_COLOR;
	eRect r_dlg(ePoint(130, 60));
	r_dlg.Move(ePoint(8, 8));
	Bound() = r_dlg;
	ePoint margin(6, 6);
	eRect r(ePoint(r_dlg.Width() - margin.x * 2, FontSize().y));
	r.Move(margin);
	for(int i = 0; i < I_COUNT; ++i)
	{
		eButton* b = new eButton;
		Insert(b);
		b->Bound() = r;
		b->Highlight(false);
		b->Id(i);
		ItemState(i, 0);
		r.Move(ePoint(0, FontSize().y));
	}
}
//=============================================================================
//	eMenuDialog::OnNotify
//-----------------------------------------------------------------------------
void eMenuDialog::OnNotify(byte n, byte from)
{
	if(n != eButton::N_PUSH)
		return;
	eInherited::OnNotify(from, id);
}

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
			eKeysDialog* d = new eKeysDialog;
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
			eKeysDialog* d = (eKeysDialog*)*childs;
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


//=============================================================================
//	eManager::Update
//-----------------------------------------------------------------------------
void eManager::Update()
{
	if(key)
	{
		if(keypress_timer > KEY_REPEAT_DELAY)
		{
			eInherited::OnKey(key, key_flags);
		}
		++keypress_timer;
	}
	eInherited::Update();
}
//=============================================================================
//	eManager::OnKey
//-----------------------------------------------------------------------------
void eManager::OnKey(char _key, dword flags)
{
	key_flags = flags;
	bool pressed = flags&xPlatform::KF_DOWN;
	if((pressed && !(flags&(xPlatform::KF_ALT|xPlatform::KF_SHIFT)) && (_key == key)) || (!pressed && (_key != key)))
		return;
	key = pressed ? _key : '\0';
	if(!key)
		keypress_timer = 0;
	eInherited::OnKey(key, flags);
}

}
//namespace xUi

#endif//USE_UI
