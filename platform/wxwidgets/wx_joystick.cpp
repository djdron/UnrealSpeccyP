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

#include "../platform.h"

#ifdef USE_WXWIDGETS

#undef self

#include "wx_joystick.h"
#include <wx/joystick.h>

#include "../../options_common.h"

namespace xPlatform
{

//=============================================================================
//	eWxJoystick::eWxJoystick
//-----------------------------------------------------------------------------
eWxJoystick::eWxJoystick(wxWindow* owner, int id) : joy(NULL)
{
#if wxUSE_JOYSTICK
	joy = new wxJoystick(id);
	joy->SetCapture(owner);
#endif//wxUSE_JOYSTICK
}
//=============================================================================
//	eWxJoystick::~eWxJoystick
//-----------------------------------------------------------------------------
eWxJoystick::~eWxJoystick()
{
#if wxUSE_JOYSTICK
	joy->ReleaseCapture();
	delete joy;
#endif//wxUSE_JOYSTICK
}
//=============================================================================
//	eWxJoystick::OnEvent
//-----------------------------------------------------------------------------
void eWxJoystick::OnEvent(wxJoystickEvent& event)
{
#if wxUSE_JOYSTICK
	if(event.GetEventType() == wxEVT_JOY_BUTTON_DOWN || event.GetEventType() == wxEVT_JOY_BUTTON_UP)
	{
		ProcessButton(JB_FIRE, event.ButtonIsDown(), 'f');
	}
	else if(event.GetEventType() == wxEVT_JOY_MOVE)
	{
		int range_x = joy->GetXMax() - joy->GetXMin();
		int range_y = joy->GetYMax() - joy->GetYMin();
		ProcessButton(JB_UP,	event.GetPosition().y < joy->GetYMin() + range_y/4, 'u');
		ProcessButton(JB_DOWN,	event.GetPosition().y > joy->GetYMax() - range_y/4, 'd');
		ProcessButton(JB_LEFT,	event.GetPosition().x < joy->GetXMin() + range_x/4, 'l');
		ProcessButton(JB_RIGHT,	event.GetPosition().x > joy->GetXMax() - range_x/4, 'r');
	}
#endif//wxUSE_JOYSTICK
}
//=============================================================================
//	eWxJoystick::ProcessButton
//-----------------------------------------------------------------------------
void eWxJoystick::ProcessButton(dword button, bool state_new, byte key)
{
#if wxUSE_JOYSTICK
	if(state.buttons[button] != state_new)
	{
		state.buttons[button] = state_new;
		if(state_new) // pressed
			Handler()->OnKey(key, KF_DOWN|OpJoyKeyFlags());
		else // released
			Handler()->OnKey(key, OpJoyKeyFlags());
	}
#endif//wxUSE_JOYSTICK
}

}
//namespace xPlatform

#endif//USE_WXWIDGETS
