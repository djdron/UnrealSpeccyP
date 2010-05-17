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

#include "desktop.h"
#include "../platform/platform.h"

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eManager::Update
//-----------------------------------------------------------------------------
void eDesktop::Update()
{
	if(key)
	{
		if(keypress_timer > KEY_REPEAT_DELAY)
		{
			eInherited::OnKey(key, key_flags);
		}
		++keypress_timer;
	}
	eInherited::Update();
}
//=============================================================================
//	eDesktop::OnKey
//-----------------------------------------------------------------------------
void eDesktop::OnKey(char _key, dword flags)
{
	key_flags = flags;
	bool pressed = flags&xPlatform::KF_DOWN;
	if((pressed && !(flags&(xPlatform::KF_ALT|xPlatform::KF_SHIFT)) && (_key == key)) || (!pressed && (_key != key)))
		return;
	key = pressed ? _key : '\0';
	if(!key)
		keypress_timer = 0;
	eInherited::OnKey(key, flags);
}

}
//namespace xUi

#endif//USE_UI
