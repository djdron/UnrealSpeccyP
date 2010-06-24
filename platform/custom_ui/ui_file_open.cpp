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

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eFileOpenDialog::eFileOpenDialog
//-----------------------------------------------------------------------------
eFileOpenDialog::eFileOpenDialog(const char* _path) : list(NULL), selected(NULL)
{
	strcpy(path, _path);
	memset(folders, 0, sizeof(folders));
}
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
//	eFileOpenDialog::Init
//-----------------------------------------------------------------------------
int eFileOpenDialog::PathLevel() const
{
	int level = 0;
	for(const char* src = path; *src; ++src)
	{
		if(*src == '\\' || *src == '/')
			++level;
	}
	return level;
}
//=============================================================================
//	eFileOpenDialog::OnChangePath
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnChangePath()
{
	list->Clear();
	memset(folders, 0, sizeof(folders));

	int i = 0;
	if(PathLevel() > 1)
	{
		list->Insert("..");
		folders[i++] = true;
	}
	for(xIo::eFileSelect fs(path); i < MAX_ITEMS && fs.Valid(); fs.Next())
	{
		if(!fs.IsFile() && !fs.IsDir())
			continue;
		if(!strcmp(fs.Name(), ".") || !strcmp(fs.Name(), ".."))
			continue;
		if(!fs.IsDir() && !xPlatform::Handler()->FileTypeSupported(fs.Name()))
			continue;
		list->Insert(fs.Name());
		folders[i++] = fs.IsDir();
	}
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
	if(!list->Item())
		return;
	if(folders[list->Selected()])
	{
		if(!strcmp(list->Item(), ".."))
		{
			GetUpLevel(path, 2);
		}
		else
		{
			GetUpLevel(path);
			strcat(path, list->Item());
			strcat(path, "/");
		}
		OnChangePath();
		return;
	}
	GetUpLevel(path);
	strcat(path, list->Item());
	selected = path;
	eInherited::OnNotify(n, id);
}

}
//namespace xUi

#endif//USE_UI
