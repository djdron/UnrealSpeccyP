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
#include "ui_button.h"

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eButton::Text
//-----------------------------------------------------------------------------
void eButton::Text(const char* s)
{
	assert(strlen(s) <= MAX_TEXT_SIZE);
	strcpy(text, s);
	changed = true;
}
//=============================================================================
//	eButton::Update
//-----------------------------------------------------------------------------
void eButton::Update()
{
	bool change_focus = focused != last_focused;
	eInherited::Update();
	if(changed)
	{
		changed = false;
		eRect sr = ScreenBound();
		DrawRect(sr, focused ? FOCUS_COLOR : background);
		ePoint cen = sr.Beg() + ePoint(sr.Width() / 2, sr.Height() / 2);
		ePoint t_half((strlen(text) * FontSize().x / 2), FontSize().y / 2);
		eRect r(cen.x - t_half.x, cen.y - t_half.y, cen.x + t_half.x, cen.y + t_half.y);
		DrawText(r, text);
	}
	if(pushed != last_pushed)
	{
		last_pushed = pushed;
		if(triggered && highlight)	DrawRect(ScreenBound(), focused ? PUSH_FOCUS_COLOR : PUSH_COLOR, 0x08ffffff);
		else			DrawRect(ScreenBound(), focused ? FOCUS_COLOR : background, 0x08ffffff);
		Notify(pushed ? N_PUSH : N_POP);
	}
	if(change_focus && triggered && highlight)
	{
		DrawRect(ScreenBound(), focused ? PUSH_FOCUS_COLOR : PUSH_COLOR, 0x08ffffff);
	}
}
//=============================================================================
//	eButton::OnKey
//-----------------------------------------------------------------------------
bool eButton::OnKey(char key, dword flags)
{
	if(last_key != key)
	{
		last_key = key;
		if((!triggered && !key) || key == 'e' || key == 'f')
		{
			Push(key);
			return true;
		}
		else if(key == ' ')
		{
			triggered = !triggered;
			pushed = triggered;
			return true;
		}
	}
	return false;
}

}
//namespace xUi

#endif//USE_UI
