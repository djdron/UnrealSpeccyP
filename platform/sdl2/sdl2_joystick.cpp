/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2022 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_SDL2
#ifdef SDL_USE_JOYSTICK

#include <SDL.h>
#include "../../tools/options.h"
#include "../../options_common.h"

namespace xPlatform
{

struct ePadState
{
	ePadState()
	{
		memset(axes, 0, sizeof(axes));
		memset(buttons, 0, sizeof(buttons));
	}
	float axes[SDL_CONTROLLER_AXIS_MAX];
	bool buttons[SDL_CONTROLLER_BUTTON_MAX];
};

static bool ButtonPressed(dword button, const ePadState& pad, const ePadState& pad_prev)
{
	return !pad_prev.buttons[button] && pad.buttons[button];
}

static void ProcessKey(bool state, bool state_prev, byte key)
{
	if(state == state_prev) // state not changed
		return;
	if(state) // pressed
		Handler()->OnKey(key, KF_DOWN|OpJoyKeyFlags());
	else // released
		Handler()->OnKey(key, OpJoyKeyFlags());
}

static void ProcessButton(dword button, const ePadState& pad, const ePadState& pad_prev, byte key)
{
	ProcessKey(pad.buttons[button], pad_prev.buttons[button], key);
}

static void ProcessAxis(dword axis, const ePadState& pad, const ePadState& pad_prev, byte key1, byte key2)
{
	bool b0 = pad_prev.axes[axis] > 0.5f;
	bool b1 = pad.axes[axis] > 0.5f;
	ProcessKey(b1, b0, key1);
	b0 = pad_prev.axes[axis] < -0.5f;
	b1 = pad.axes[axis] < -0.5f;
	ProcessKey(b1, b0, key2);
}

void ProcessJoy(SDL_Event& e)
{
	static ePadState pad, pad_prev;
	switch(e.type)
	{
	case SDL_CONTROLLERBUTTONDOWN:
		if(e.cbutton.button != SDL_CONTROLLER_BUTTON_INVALID && e.cbutton.button < SDL_CONTROLLER_BUTTON_MAX)
			pad.buttons[e.cbutton.button] = true;
		break;
	case SDL_CONTROLLERBUTTONUP:
		if(e.cbutton.button != SDL_CONTROLLER_BUTTON_INVALID && e.cbutton.button < SDL_CONTROLLER_BUTTON_MAX)
			pad.buttons[e.cbutton.button] = false;
		break;
	case SDL_CONTROLLERAXISMOTION:
		if(e.caxis.axis != SDL_CONTROLLER_AXIS_INVALID && e.caxis.axis < SDL_CONTROLLER_AXIS_MAX)
			pad.axes[e.caxis.axis] = float(e.caxis.value)/32768;
		break;
	default:
		break;
	}
	ProcessButton(SDL_CONTROLLER_BUTTON_DPAD_UP,	pad, pad_prev, 'u');
	ProcessButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN,	pad, pad_prev, 'd');
	ProcessButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT,	pad, pad_prev, 'l');
	ProcessButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT,	pad, pad_prev, 'r');
	ProcessButton(SDL_CONTROLLER_BUTTON_A,			pad, pad_prev, 'f');
	ProcessButton(SDL_CONTROLLER_BUTTON_B,			pad, pad_prev, 'u');
	ProcessButton(SDL_CONTROLLER_BUTTON_X,			pad, pad_prev, 'e');
	ProcessButton(SDL_CONTROLLER_BUTTON_Y,			pad, pad_prev, ' ');
	ProcessButton(SDL_CONTROLLER_BUTTON_BACK,		pad, pad_prev, 'm');
	ProcessButton(SDL_CONTROLLER_BUTTON_START,		pad, pad_prev, 'k');

	ProcessAxis(SDL_CONTROLLER_AXIS_LEFTX,			pad, pad_prev, 'r', 'l');
	ProcessAxis(SDL_CONTROLLER_AXIS_LEFTY,			pad, pad_prev, 'd', 'u');
	ProcessAxis(SDL_CONTROLLER_AXIS_RIGHTX,			pad, pad_prev, 'r', 'l');
	ProcessAxis(SDL_CONTROLLER_AXIS_RIGHTY,			pad, pad_prev, 'd', 'u');
//	ProcessAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,	pad, pad_prev, 'r', 'l');
//	ProcessAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,	pad, pad_prev, 'd', 'u');

	if(ButtonPressed(SDL_CONTROLLER_BUTTON_LEFTSTICK, pad, pad_prev) || ButtonPressed(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, pad, pad_prev))
		Handler()->OnAction(A_RESET);
	if(ButtonPressed(SDL_CONTROLLER_BUTTON_RIGHTSTICK, pad, pad_prev) || ButtonPressed(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, pad, pad_prev))
	{
		using namespace xOptions;
		eOptionB* o = eOptionB::Find("pause");
		SAFE_CALL(o)->Change();
	}
	if(pad.buttons[SDL_CONTROLLER_BUTTON_BACK] && pad.buttons[SDL_CONTROLLER_BUTTON_START])
		OpQuit(true);
	pad_prev = pad;
}

}
//namespace xPlatform

#endif//SDL_USE_JOYSTICK
#endif//USE_SDL2
