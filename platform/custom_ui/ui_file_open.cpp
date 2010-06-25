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
//	eFileOpenDialog::OnChangePath
//-----------------------------------------------------------------------------
void eFileOpenDialog::OnChangePath()
{
	list->Clear();
	memset(folders, 0, sizeof(folders));

	int i = 0;
	if(!xIo::PathIsRoot(path))
	{
		list->Insert("..");
		folders[i++] = true;
	}
	// put folder first
	for(xIo::eFileSelect ds(path); i < MAX_ITEMS && ds.Valid(); ds.Next())
	{
		if(!ds.IsDir())
			continue;
		if(!strcmp(ds.Name(), ".") || !strcmp(ds.Name(), ".."))
			continue;
		list->Insert(ds.Name());
		folders[i++] = true;
	}
	for(xIo::eFileSelect fs(path); i < MAX_ITEMS && fs.Valid(); fs.Next())
	{
		if(!fs.IsFile() || !xPlatform::Handler()->FileTypeSupported(fs.Name()))
			continue;
		list->Insert(fs.Name());
		folders[i++] = false;
	}
}
//=============================================================================
//	GetUpLevel
//-----------------------------------------------------------------------------
static const char* GetUpLevel(char* path)
{
	static char level_name[xIo::MAX_PATH_LEN];
	level_name[0] = '\0';
	int l = strlen(path);
	if(!l)
		return level_name;
	char* path_end = path + l - 1;
	*path_end = '\0';
	while(path_end > path)
	{
		--path_end;
		if(*path_end == '\\' || *path_end == '/')
		{
			++path_end;
			strcpy(level_name, path_end);
			*path_end = '\0';
			return level_name;
		}
	}
	strcpy(level_name, path);
	*path = '\0';
	return level_name;
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
		const char* item = NULL;
		if(!strcmp(list->Item(), ".."))
		{
			item = GetUpLevel(path);
		}
		else
		{
			strcat(path, list->Item());
			strcat(path, "/");
		}
		OnChangePath();
		if(item)
			list->Item(item);
		return;
	}
	strcat(path, list->Item());
	selected = path;
	eInherited::OnNotify(n, id);
}

}
//namespace xUi

#endif//USE_UI
