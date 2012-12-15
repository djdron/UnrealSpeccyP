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
#include "../../ui/ui_tree.h"
#include "../../tools/options.h"
#include "ui_menu.h"

#ifdef USE_UI

namespace xUi
{

using namespace xOptions;

void eMenu::Init()
{
	background = COLOR_BACKGROUND;
	eRect r(8, 8, 130, 150);
	ePoint margin(6, 6);
	Bound() = r;
	tree = new eTree;
	tree->Bound() = eRect(margin.x, margin.y, r.Width() - margin.x, r.Height() - margin.y);
	Insert(tree);
	OnChangeOption();
}
void eMenu::CreateOption(int& i, eOptionB* o)
{
	const char* value = NULL;
	if(o->Customizable())
	{
		value = o->Value();
	}
	else if(!o->SubOptions())
		return;
	eTree::eItemInserter tii(*tree, o->Name(), value);
	options[i++] = o;
	for(eOptionB* so = o->SubOptions(); so; so = so->Next())
	{
		CreateOption(i, so);
	}
}
void eMenu::OnChangeOption()
{
	tree->Clear(true);
	int i = 0;
	for(eRootOptionB* o = eRootOptionB::First(); o; o = o->Next())
	{
		CreateOption(i, *o);
	}
}
bool eMenu::ChangeOption(bool next)
{
	int item = tree->Selected();
	if(item == -1)
		return false;
	options[item]->Change(next);
	eRootOptionB::Apply();
	OnChangeOption();
	return true;
}
bool eMenu::OnKey(char key, dword flags)
{
	eInherited::OnKey(key, flags);
	switch(key)
	{
	case 'l': return ChangeOption(false);
	case 'r':
	case 'f':
	case 'e':
	case ' ': return ChangeOption(true);
	}
	return false;
}

}
//namespace xUi

#endif//USE_UI
