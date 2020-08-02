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

#ifdef RG350
#include "../../speccy.h"
#endif

#ifdef USE_UI

namespace xUi
{

using namespace xOptions;

class eOptionButton : public eButton
{
	typedef eButton eInherited;
public:
	eOptionButton(eOptionB* o) { option = o; }
	virtual bool OnKey(char key, dword flags)
	{
		if(eInherited::OnKey(key, flags))
			return true;
		if(!option->Values())
			return false;
		switch(key)
		{
		case 'l':	Change(false);	return true;
		case 'r':	Change(true);	return true;
		}
		return false;
	}
	void Change(bool next = true)
	{
		option->Change(next);
		UpdateText();
	}
	void UpdateText()
	{
		char text[128];
		strcpy(text, option->Name());
		int offs = strlen(text);
		int spc_count = Bound().Width() / FontSize().x - offs;
		const char* state = option->Value();
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
		Text(text);
	}
	eOptionB* option;
};

static int last_selected = 0;

//=============================================================================
//	eMenu::~eMenu
//-----------------------------------------------------------------------------
eMenu::~eMenu()
{
	if(focused)
	{
		eControl* const * c = Childs();
		for(; *c && *c != focused; ++c) {}
		last_selected = c - Childs();
	}
}

//=============================================================================
//	eMenu::Init
//-----------------------------------------------------------------------------
void eMenu::Init()
{
	background = COLOR_BACKGROUND;
	ePoint margin(6, 6);
	#ifdef RG350
		eRect r_dlg(ePoint(130, 2*margin.y));
		r_dlg.Move(ePoint(32, 24));
	#else
		eRect r_dlg(ePoint(160, 2*margin.y));
		r_dlg.Move(ePoint(8, 8));
	#endif
	eRect r(ePoint(r_dlg.Width() - margin.x * 2, margin.y));
	r.Move(margin);
	byte i = 0;
	for(eOptionB* o = eOptionB::First(); o; o = o->Next())
	{
		if(!o->Customizable())
			continue;
		eOptionButton* b = new eOptionButton(o);
		Insert(b);
		b->Bound() = r;
		b->Highlight(false);
		b->Id(i);
		r.Move(ePoint(0, FontSize().y));
		r_dlg.bottom += FontSize().y;
		b->UpdateText();
		++i;
	}
	Bound() = r_dlg;
	if(focused)
		focused->Focused(false);
	focused = Childs()[last_selected];
		focused->Focused(true);
}
//=============================================================================
//	eMenu::OnNotify
//-----------------------------------------------------------------------------
void eMenu::OnNotify(byte n, byte from)
{
	if(n == eButton::N_PUSH)
	{
		eOptionButton* b = (eOptionButton*)childs[from];
		b->Change();
	}
}

}
//namespace xUi

#endif//USE_UI
