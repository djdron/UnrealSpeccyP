/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
#include "ui_web_browse.h"
#include "../../ui/ui_list.h"
#include "../../ui/ui_desktop.h"
#include "../../tools/options.h"
#include "../../tools/io_select.h"

#ifdef USE_UI
#ifdef USE_WEB

static struct eOptionWebBrowseLastItem : public xOptions::eOptionString
{
	eOptionWebBrowseLastItem() { customizable = false; }
	virtual const char* Name() const { return "web browse last item"; }
	const char* Path()
	{
		static char path[xIo::MAX_PATH_LEN];
		strcpy(path, Value());
		int l = strlen(path);
		if(!l || path[l - 1] == '\\' || path[l - 1] == '/')
			return path;
		xIo::GetPathParent(path, Value());
		strcat(path, "/");
		return path;
	}
} op_web_browse_last_item;

namespace xIo
{
eFileSelect* FileSelectWEB(const char* path);
const char* FileSelectWEB_Open(const char* name);
}

namespace xUi
{

//=============================================================================
//	eWebBrowseDialog::eWebBrowseDialog
//-----------------------------------------------------------------------------
eWebBrowseDialog::eWebBrowseDialog() : list(NULL), selected(NULL)
{
	strcpy(path, op_web_browse_last_item.Path());
}
//=============================================================================
//	eWebBrowseDialog::Init
//-----------------------------------------------------------------------------
void eWebBrowseDialog::Init()
{
	background = COLOR_BACKGROUND;
	eRect r(8, 8, 120, 180);
	ePoint margin(6, 6);
	Bound() = r;
	list = new eList;
	list->Bound() = eRect(margin.x, margin.y, r.Width() - margin.x, r.Height() - margin.y);
	Insert(list);
	OnChangePath();
	int l = strlen(op_web_browse_last_item.Path());
	if(l)
	{
		list->Item(op_web_browse_last_item.Value() + l);
	}
}
//=============================================================================
//	eWebBrowseDialog::OnChangePath
//-----------------------------------------------------------------------------
void eWebBrowseDialog::OnChangePath()
{
	list->Clear();
	if(strlen(path))
		list->Insert("/..");
	xIo::eFileSelect* fs = xIo::FileSelectWEB(path);
	for(; fs->Valid(); fs->Next())
	{
		if(fs->IsDir())
		{
			char name[xIo::MAX_PATH_LEN];
			strcpy(name, "/");
			strcat(name, fs->Name());
			list->Insert(name);
		}
		else
			list->Insert(fs->Name());
	}
}
//=============================================================================
//	eWebBrowseDialog::OnNotify
//-----------------------------------------------------------------------------
void eWebBrowseDialog::OnNotify(byte n, byte from)
{
	if(!list->Item())
		return;

	eControl* p = Parent();
	for(; p->Parent(); p = p->Parent()) {}
	((eDesktop*)p)->KillAutorepeat();

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
			if(strlen(path)) // not root item
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
	op_web_browse_last_item.Value(path);
	selected = xIo::FileSelectWEB_Open(path);
	eInherited::OnNotify(n, id);
	list->Clear();
}

}
//namespace xUi

#endif//USE_WEB
#endif//USE_UI
