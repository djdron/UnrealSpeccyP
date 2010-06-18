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
#include "ui_dialog.h"

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eDialog::Insert
//-----------------------------------------------------------------------------
void eDialog::Insert(eControl* child)
{
	changed = true;
	for(int i = 0; i < MAX_CHILDS; ++i)
	{
		if(childs[i])
			continue;
		child->Parent(this);
		child->Background() = background;
		child->Init();
		childs[i] = child;
		childs[i + 1] = NULL;
		break;
	}
}
//=============================================================================
//	eDialog::Update
//-----------------------------------------------------------------------------
void eDialog::Update()
{
	if(changed)
	{
		changed = false;
		DrawRect(bound, background);
		focused = *childs;
		SAFE_CALL(focused)->Focused(true);
	}
	for(int i = 0; childs[i]; ++i)
	{
		childs[i]->Update();
	}
}
//=============================================================================
//	eDialog::ChooseFocus
//-----------------------------------------------------------------------------
void eDialog::ChooseFocus(char key)
{
	if(!focused)
		return;
	ePoint org = focused->Bound().Beg();
	eControl* f = NULL;
	ePoint fp;
	for(int i = 0; i < MAX_CHILDS; ++i)
	{
		if(!childs[i])
			break;
		const ePoint& p = childs[i]->Bound().Beg();
		if(((key == 'l' || key == 'r') && p.y != org.y)
			|| ((key == 'u' || key == 'd') && p.x != org.x))
			continue;
		if((key == 'l' && p.x < org.x && (!f || p.x > fp.x))
			|| (key == 'r' && p.x > org.x && (!f || p.x < fp.x))
			|| (key == 'u' && p.y < org.y && (!f || p.y > fp.y))
			|| (key == 'd' && p.y > org.y && (!f || p.y < fp.y)))
		{
			f = childs[i];
			fp = f->Bound().Beg();
		}
	}
	if(f)
	{
		focused->Focused(false);
		focused = f;
		focused->Focused(true);
	}
}
//=============================================================================
//	eDialog::OnKey
//-----------------------------------------------------------------------------
bool eDialog::OnKey(char key, dword flags)
{
	if(focused && focused->OnKey(key, flags))
			return true;
	switch(key)
	{
	case 'l':
	case 'r':
	case 'u':
	case 'd':
		ChooseFocus(key);
		return true;
	}
	return false;
}

}
//namespace xUi

#endif//USE_UI
