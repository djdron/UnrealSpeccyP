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

#include "../platform.h"
#include "ui_keyboard.h"
#include "../../ui/ui_button.h"

#ifdef USE_UI

namespace xUi
{

static const char* zx_keys[] =
{
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
	"A", "S", "D", "F", "G", "H", "J", "K", "L", "En",
	"Cs", "Z", "X", "C", "V", "B", "N", "M", "Ss", "Sp"
};

//=============================================================================
//	eKeysDialog::AllocateId
//-----------------------------------------------------------------------------
byte eKeyboard::AllocateId(const char* key) const
{
	if(strlen(key) == 1)	return *key;
	if(!strcmp(key, "Cs"))	return 'c';
	if(!strcmp(key, "Ss"))	return 's';
	if(!strcmp(key, "En"))	return 'e';
	if(!strcmp(key, "Sp"))	return ' ';
	return -1;
}
//=============================================================================
//	eKeyboard::Init
//-----------------------------------------------------------------------------
void eKeyboard::Init()
{
	background = COLOR_BACKGROUND;
	eRect r_dlg(ePoint(169, 70));
	r_dlg.Move(ePoint(143, 8));
	Bound() = r_dlg;
	eRect r_item(ePoint(13, FontSize().y + 2));
	ePoint margin(6, 6);
	ePoint delta;
	delta.x = (r_dlg.Width() - r_item.Width() - margin.x * 2) / 9;
	delta.y = (r_dlg.Height() - r_item.Height() - margin.y * 2) / 3;
	r_item.Move(margin);
	for(int col = 0; col < 4; ++col)
	{
		eRect r = r_item;
		for(int row = 0; row < 10; ++row)
		{
			eButton* b = new eButton;
			b->Bound() = r;
			const char* s = zx_keys[col * 10 + row];
			b->Id(AllocateId(s));
			b->Text(s);
			Insert(b);
			r.Move(ePoint(delta.x, 0));
		}
		r_item.Move(ePoint(0, delta.y));
	}
}
//=============================================================================
//	eKeyboard::OnKey
//-----------------------------------------------------------------------------
bool eKeyboard::OnKey(char key, dword _flags)
{
	using namespace xPlatform;
	if((_flags&KF_SHIFT) != (flags&KF_SHIFT))
		((eButton*)childs[30])->Push(_flags&KF_SHIFT);
	if((_flags&KF_ALT) != (flags&KF_ALT))
		((eButton*)childs[38])->Push(_flags&KF_ALT);
	flags = _flags;
	return eInherited::OnKey(key, flags);
}
//=============================================================================
//	eKeyboard::OnNotify
//-----------------------------------------------------------------------------
void eKeyboard::OnNotify(byte n, byte from)
{
	bool pushed = n == eButton::N_PUSH;
	switch(from)
	{
	case 'c': caps = pushed;	break;
	case 's': symbol = pushed;	break;
	default:
		key = from;
		pressed = pushed;
	}
	eInherited::OnNotify(n, id);
}

}
//namespace xUi

#endif//USE_UI
