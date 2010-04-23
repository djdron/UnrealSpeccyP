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
#include "controls.h"

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eButton::Update
//-----------------------------------------------------------------------------
void eButton::Update()
{
	if(changed)
	{
		changed = false;
		eRect sr = ScreenBound();
		ePoint cen = sr.Beg() + ePoint(sr.Width() / 2, sr.Height() / 2);
		ePoint t_half((strlen(text) * FontSize().x / 2), FontSize().y / 2);
		eRect r(cen.x - t_half.x, cen.y - t_half.y, cen.x + t_half.x, cen.y + t_half.y);
		DrawText(r, text);
	}
}
//=============================================================================
//	eButton::OnKey
//-----------------------------------------------------------------------------
void eButton::OnKey(char key)
{
	Notify(key ? N_PUSH : N_POP, id);
}

//=============================================================================
//	eList::Insert
//-----------------------------------------------------------------------------
void eList::Insert(const char* item)
{
	changed = true;
	for(int i = 0; i < MAX_ITEMS; ++i)
	{
		if(items[i])
			continue;
		char* s = new char[strlen(item) + 1];
		memcpy(s, item, strlen(item) + 1);
		items[i] = s;
		items[i + 1] = NULL;
		size = i + 1;
		break;
	}
}
//=============================================================================
//	eList::Update
//-----------------------------------------------------------------------------
void eList::Update()
{
	eRect sr = ScreenBound();
	if(changed)
	{
		DrawRect(sr, background);
		eRect r(sr.left, sr.top, sr.right, sr.top + FontSize().y);
		int i = page_begin;
		for(; items[i]; ++i)
		{
			if(r.bottom > sr.bottom)
				break;
			DrawText(r, items[i]);
			r.Move(ePoint(0, FontSize().y));
		}
		page_size = i - page_begin;
	}
	if((changed || (selected != last_selected)) && page_size)
	{
		eRect cursor(sr.left, 0, sr.right, 0);
		cursor.top = sr.top + (last_selected - page_begin) * FontSize().y;
		cursor.bottom = cursor.top + FontSize().y;
		DrawRect(cursor, background, 0x08ffffff);
		last_selected = selected;
		if(selected < page_begin)
		{
			page_begin = selected;
			changed = true;
			return;
		}
		if(selected >= page_begin + page_size)
		{
			page_begin = selected - page_size + 1;
			changed = true;
			return;
		}
		cursor.top = sr.top + (selected - page_begin) * FontSize().y;
		cursor.bottom = cursor.top + FontSize().y;
		DrawRect(cursor, CURSOR_COLOR, 0x08ffffff);
	}
	changed = false;
}
//=============================================================================
//	eList::OnKey
//-----------------------------------------------------------------------------
void eList::OnKey(char key)
{
	switch(key)
	{
	case 'u': --selected; break;
	case 'd': ++selected; break;
	}
	if(selected < 0) selected = size - 1;
	if(selected >= size) selected = 0;
}

}
//namespace xUi

#endif//USE_UI
