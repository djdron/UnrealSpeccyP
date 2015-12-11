/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifndef __WX_JOYSTICK_H__
#define __WX_JOYSTICK_H__

#pragma once

#ifdef USE_WXWIDGETS

class wxWindow;
class wxJoystick;
class wxJoystickEvent;

namespace xPlatform
{

//=============================================================================
//	eWxJoystick
//-----------------------------------------------------------------------------
class eWxJoystick
{
public:
	eWxJoystick(wxWindow* owner, int id);
	~eWxJoystick();
	void OnEvent(wxJoystickEvent& event);

private:
	enum eButtonId { JB_LEFT, JB_UP, JB_RIGHT, JB_DOWN, JB_FIRE, JB_COUNT };
	struct eState
	{
		eState() { memset(buttons, 0, sizeof(buttons)); }
		bool buttons[JB_COUNT];
	};
	void ProcessButton(dword button, bool state_new, byte key);

private:
	eState state;
	wxJoystick* joy;
};

}//namespace xPlatform

#endif//USE_WXWIDGETS

#endif//__WX_JOYSTICK_H__
