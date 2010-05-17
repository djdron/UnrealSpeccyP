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
#include "file_open.h"
#include "../../ui/list.h"
#include "../io.h"

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

void GetUpLevel(char* path, int level)
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

}
//namespace xUi

#endif//USE_UI
