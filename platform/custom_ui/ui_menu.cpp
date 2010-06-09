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
#include "ui_menu.h"
#include "../../ui/ui_button.h"
#include "../../tools/options.h"


#ifdef USE_UI

namespace xUi
{

using namespace xOptions;

//=============================================================================
//	eMenu::UpdateItem
//-----------------------------------------------------------------------------
void eMenu::UpdateItem(eButton* b, eOption* o)
{
	char text[128];
	strcpy(text, o->Name());
	int offs = strlen(text);
	int spc_count = b->Bound().Width() / FontSize().x - offs;
	const char* state = o->Value();
	spc_count -= state ? strlen(state) : 0;
	for(int i = 0; i < spc_count; ++i)
	{
		text[offs++] = ' ';
	}
	text[offs] = '\0';
	if(state)
	{
		strcat(text, state);
	}
	b->Text(text);
}
//=============================================================================
//	eMenu::ChangeItem
//-----------------------------------------------------------------------------
void eMenu::ChangeItem(byte id)
{
	eOption* o = eOption::First();
	byte i = 0;
	for(; o; o = o->Next())
	{
		if(!o->Customizable())
			continue;
		if(id == i)
			break;
		++i;
	}
	assert(o);
	o->Change();
	eButton* b = (eButton*)childs[id];
	UpdateItem(b, o);
}
//=============================================================================
//	eMenu::Init
//-----------------------------------------------------------------------------
void eMenu::Init()
{
	background = BACKGROUND_COLOR;
	eRect r_dlg(ePoint(130, 70));
	r_dlg.Move(ePoint(8, 8));
	Bound() = r_dlg;
	ePoint margin(6, 6);
	eRect r(ePoint(r_dlg.Width() - margin.x * 2, FontSize().y));
	r.Move(margin);
	byte i = 0;
	for(eOption* o = eOption::First(); o; o = o->Next())
	{
		if(!o->Customizable())
			continue;
		eButton* b = new eButton;
		Insert(b);
		b->Bound() = r;
		b->Highlight(false);
		b->Id(i);
		r.Move(ePoint(0, FontSize().y));
		UpdateItem(b, o);
		++i;
	}
}
//=============================================================================
//	eMenu::OnNotify
//-----------------------------------------------------------------------------
void eMenu::OnNotify(byte n, byte from)
{
	if(n != eButton::N_PUSH)
		return;
	eInherited::OnNotify(from, id);
}

}
//namespace xUi

#endif//USE_UI
