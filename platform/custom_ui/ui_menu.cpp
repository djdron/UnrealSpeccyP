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

#ifdef USE_UI

namespace xUi
{

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
//	eMenuDialog::ItemState
//-----------------------------------------------------------------------------
void eMenuDialog::ItemState(int idx, int v)
{
	char s[80];
	GetItemText(idx, v, s);
	((eButton*)childs[idx])->Text(s);
}

}
//namespace xUi

#endif//USE_UI
