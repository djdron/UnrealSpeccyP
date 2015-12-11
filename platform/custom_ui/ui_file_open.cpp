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
#include "ui_file_open.h"
#include "../../ui/ui_list.h"
#include "../io.h"
#include "../../tools/io_select.h"
#include "../platform.h"
#include "../../options_common.h"
#include "../../file_type.h"
#include <ctype.h>

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eFileOpenDialog::eFileOpenDialog
//-----------------------------------------------------------------------------
eFileOpenDialog::eFileOpenDialog(const char* _path) : list(NULL), selected(NULL)
{
	strcpy(path, _path);
}
//=============================================================================
//	eFileOpenDialog::Init
//-----------------------------------------------------------------------------
void eFileOpenDialog::Init()
{
	background = COLOR_BACKGROUND;
	eRect r(8, 8, 120, 180);
	ePoint margin(6, 6);
	Bound() = r;
	list = new eList;
	list->Bound() = eRect(margin.x, margin.y, r.Width() - margin.x, r.Height() - margin.y);
	Insert(list);
	OnChangePath();
	int l = strlen(xPlatform::OpLastFolder());
	if(l)
	{
		list->Item(xPlatform::OpLastFile() + l);
	}
}
//=============================================================================
//	StrCaseCmp
//-----------------------------------------------------------------------------
static int StrCaseCmp(const char* a, const char* b)
{
	while(tolower(*a) == tolower(*b))
	{
		if(*a == 0)
			return 0;
		++a;
		++b;
	}
	return tolower(*a) - tolower(*b);
}
static int NameCmp(const void* _a, const void* _b)
{
	return StrCaseCmp(*(const char**)_a, *(const char**)_b);
}
//=============================================================================
//	eFileOpenDialog::OnChangePath
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnChangePath()
{
	list->Clear();

	int i = 0;
	int skip_sort = 0;
	if(!xIo::PathIsRoot(path))
	{
		list->Insert("/..");
		skip_sort = 1; // do not sort this item
	}

	xIo::eFileSelect* fs = NULL;
	for(const xPlatform::eFileType* t = xPlatform::eFileType::First(); t && !fs; t = t->Next())
	{
		fs = t->FileSelect(path);
	}
	if(!fs)
		fs = xIo::FileSelect(path);

	for(; i < MAX_ITEMS && fs->Valid(); fs->Next())
	{
		if(!strcmp(fs->Name(), ".") || !strcmp(fs->Name(), ".."))
			continue;
		if(fs->IsFile() && !xPlatform::Handler()->FileTypeSupported(fs->Name()))
			continue;
		bool dir = fs->IsDir();
		if(!dir)
		{
			char name[xIo::MAX_PATH_LEN];
			strcpy(name, path);
			strcat(name, fs->Name());
			strcat(name, "/");
			for(const xPlatform::eFileType* t = xPlatform::eFileType::First(); t; t = t->Next())
			{
				char contain_path[xIo::MAX_PATH_LEN];
				char contain_name[xIo::MAX_PATH_LEN];
				if(t->Contain(name, contain_path, contain_name))
				{
					dir = strlen(contain_name) == 0;
					break;
				}
			}
		}
		if(dir)
		{
			char name[xIo::MAX_PATH_LEN];
			strcpy(name, "/");
			strcat(name, fs->Name());
			list->Insert(name);
		}
		else
			list->Insert(fs->Name());
	}
	qsort(list->Items() + skip_sort, list->Size() - skip_sort, sizeof(const char*), NameCmp);
	delete fs;
}
//=============================================================================
//	eFileOpenDialog::OnNotify
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnNotify(byte n, byte from)
{
	if(!list->Item())
		return;
	if(list->Item()[0] == '/')
	{
		const char* select_item = NULL;
		char item[xIo::MAX_PATH_LEN];
		if(!strcmp(list->Item(), "/.."))
		{
			char parent[xIo::MAX_PATH_LEN];
			xIo::GetPathParent(parent, path);
			strcpy(item + 1, path + strlen(parent));
			item[strlen(item + 1)] = 0; // remove last /
			strcpy(path, parent);
			if(!xIo::PathIsRoot(path))
			{
				select_item = item + 1;
				strcat(path, "/");
			}
			else
			{
				item[0] = '/';
				select_item = item;
			}
		}
		else
		{
			strcat(path, list->Item() + 1);
			strcat(path, "/");
		}
		OnChangePath();
		if(select_item)
			list->Item(select_item);
		return;
	}
	strcat(path, list->Item());
	selected = path;
	eInherited::OnNotify(n, id);
}

}
//namespace xUi

#endif//USE_UI
