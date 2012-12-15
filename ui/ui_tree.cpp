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
#include "ui_tree.h"

#ifdef USE_UI

namespace xUi
{

void eTree::Clear(bool items_only)
{
	changed = true;
	for(eItem** o = items; *o; ++o)
	{
		SAFE_DELETE_ARRAY((*o)->id);
		SAFE_DELETE_ARRAY((*o)->value);
		SAFE_DELETE(*o);
	}
	*items = NULL;
	sub_level = size = 0;
	if(!items_only)
	{
		page_begin = page_size = 0;
		last_selected = selected = -1;
	}
}
void eTree::Insert(const char* _id, const char* _value)
{
	eItem* i = new eItem;
	char* id = new char[strlen(_id) + 1];
	memcpy(id, _id, strlen(_id) + 1);
	i->id = id;
	char* value = NULL;
	if(_value)
	{
		value = new char[strlen(_value) + 1];
		memcpy(value, _value, strlen(_value) + 1);
	}
	i->value = value;
	i->sub_level = sub_level;
	items[size] = i;
	items[size + 1] = NULL;
	++size;
	changed = true;
	if(selected < 0)
		selected = 0;
}
void eTree::Update()
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
			r.left = r.right = sr.right;
			if(items[i]->value)
			{
				int l_max = sr.Width() / FontSize().x;
				int l = strlen(items[i]->value);
				int l_clip = l > l_max ? l_max : l;
				const char* value = items[i]->value + l - l_clip;
				r.left = sr.right - l_clip * FontSize().x;
				DrawText(r, value);
				r.left -= 6;
			}
			int x = sr.left + items[i]->sub_level * FontSize().x;
			if(x < r.left)
			{
				int l_clip = (r.left - x) / FontSize().x;
				r.left = x;
				r.right = x + l_clip * FontSize().x;
				DrawText(r, items[i]->id);
			}
			r.Move(ePoint(0, FontSize().y));
		}
		page_size = i - page_begin;
	}
	if((changed || (selected != last_selected)) && page_size)
	{
		eRect cursor(sr.left, 0, sr.right, 0);
		cursor.top = sr.top + (last_selected - page_begin) * FontSize().y;
		cursor.bottom = cursor.top + FontSize().y;
		DrawRect(cursor, background, COLOR_WHITE);
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
		DrawRect(cursor, COLOR_CURSOR, COLOR_WHITE);
	}
	changed = false;
}
void eTree::Selected(int s)
{
	if(!size)
	{
		selected = -1;
		return;
	}
	if(s == -1 && s == selected - 1)
		selected = size - 1;
	else if(s == size && s == selected + 1)
		selected = 0;
	else if(s >= size)
		selected = size - 1;
	else if(s <= 0)
		selected = 0;
	else
		selected = s;
}
bool eTree::OnKey(char key, dword flags)
{
	switch(key)
	{
	case 'u': Selected(selected - 1);			return true;
	case 'd': Selected(selected + 1);			return true;
	case 'f':
	case 'e':
	case ' ': Notify(N_SELECTED);				return true;
	}
	return false;
}

}
//namespace xUi

#endif//USE_UI
