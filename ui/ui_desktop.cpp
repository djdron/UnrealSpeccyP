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

#include "ui_desktop.h"
#include "../platform/platform.h"

#ifdef USE_UI

namespace xUi
{

//=============================================================================
//	eManager::Update
//-----------------------------------------------------------------------------
void eDesktop::Update()
{
	if(autorepeat.Ok())
	{
		autorepeat.Repeat();
		eInherited::OnKey(autorepeat.key, autorepeat.key_flags);
	}
	eInherited::Update();
}
//=============================================================================
//	eDesktop::OnKey
//-----------------------------------------------------------------------------
bool eDesktop::OnKey(char key, dword flags)
{
	bool pressed = flags&xPlatform::KF_DOWN;
	if((pressed && !(flags&(xPlatform::KF_ALT|xPlatform::KF_SHIFT)) && (key == autorepeat.key)) || (!pressed && (key != autorepeat.key)))
		return false;
	if(!pressed)
		key = '\0';
	autorepeat.Set(key, flags);
	return eInherited::OnKey(key, flags);
}

}
//namespace xUi

#endif//USE_UI
