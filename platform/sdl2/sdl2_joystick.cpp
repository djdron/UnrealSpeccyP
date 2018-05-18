/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2018 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

enum ePadButton
{
	PB_A, PB_B, PB_X, PB_Y,
	PB_LTRIGGER, PB_RTRIGGER,
	PB_SELECT, PB_START,
	PB_LSHIFT, PB_RSHIFT,
	PB_STICK1, PB_STICK2,
	PB_UP, PB_DOWN, PB_LEFT, PB_RIGHT,
	PB_SYSTEM, PB_COUNT
};
enum { AXES_COUNT = 8 };
enum eHatButton
{
	HAT_UP, HAT_DOWN, HAT_LEFT, HAT_RIGHT,
	HAT_COUNT
};

struct ePadState
{
	ePadState()
	{
		memset(axes, 0, sizeof(axes));
		memset(buttons, 0, sizeof(buttons));
		memset(hat, 0, sizeof(hat));
	}
	float axes[AXES_COUNT];
	bool buttons[PB_COUNT];
	bool hat[HAT_COUNT];
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

static void ProcessHat(dword button, const ePadState& pad, const ePadState& pad_prev, byte key)
{
	ProcessKey(pad.hat[button], pad_prev.hat[button], key);
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
	case SDL_JOYBUTTONDOWN:
		if(e.jbutton.button < PB_COUNT)
			pad.buttons[e.jbutton.button] = true;
		break;
	case SDL_JOYBUTTONUP:
		if(e.jbutton.button < PB_COUNT)
			pad.buttons[e.jbutton.button] = false;
		break;
	case SDL_JOYAXISMOTION:
		if(e.jaxis.axis < AXES_COUNT)
			pad.axes[e.jaxis.axis] = float(e.jaxis.value)/32768;
		break;
	case SDL_JOYHATMOTION:
		pad.hat[HAT_UP] = (e.jhat.value & SDL_HAT_UP) != 0;
		pad.hat[HAT_DOWN] = (e.jhat.value & SDL_HAT_DOWN) != 0;
		pad.hat[HAT_LEFT] = (e.jhat.value & SDL_HAT_LEFT) != 0;
		pad.hat[HAT_RIGHT] = (e.jhat.value & SDL_HAT_RIGHT) != 0;
		break;
	default:
		break;
	}
	ProcessButton(PB_UP, pad, pad_prev, 'u');
	ProcessButton(PB_DOWN, pad, pad_prev, 'd');
	ProcessButton(PB_LEFT, pad, pad_prev, 'l');
	ProcessButton(PB_RIGHT, pad, pad_prev, 'r');
	ProcessButton(PB_A, pad, pad_prev, 'f');
	ProcessButton(PB_B, pad, pad_prev, 'e');
	ProcessButton(PB_X, pad, pad_prev, '1');
	ProcessButton(PB_Y, pad, pad_prev, ' ');
	ProcessButton(PB_SELECT, pad, pad_prev, 'm');
	ProcessButton(PB_START, pad, pad_prev, 'k');

	ProcessAxis(0, pad, pad_prev, 'r', 'l');
	ProcessAxis(1, pad, pad_prev, 'd', 'u');
	ProcessAxis(3, pad, pad_prev, 'r', 'l');
	ProcessAxis(4, pad, pad_prev, 'd', 'u');
	ProcessAxis(6, pad, pad_prev, 'r', 'l');
	ProcessAxis(7, pad, pad_prev, 'd', 'u');

	ProcessHat(HAT_UP, pad, pad_prev, 'u');
	ProcessHat(HAT_DOWN, pad, pad_prev, 'd');
	ProcessHat(HAT_LEFT, pad, pad_prev, 'l');
	ProcessHat(HAT_RIGHT, pad, pad_prev, 'r');

	if(ButtonPressed(PB_LSHIFT, pad, pad_prev) || ButtonPressed(PB_LTRIGGER, pad, pad_prev))
		Handler()->OnAction(A_RESET);
	if(ButtonPressed(PB_RSHIFT, pad, pad_prev) || ButtonPressed(PB_RTRIGGER, pad, pad_prev))
	{
		using namespace xOptions;
		eOptionB* o = eOptionB::Find("pause");
		SAFE_CALL(o)->Change();
	}
	if(pad.buttons[PB_SELECT] && pad.buttons[PB_START])
		OpQuit(true);
	pad_prev = pad;
}

}
//namespace xPlatform

#endif//SDL_USE_JOYSTICK
#endif//USE_SDL2
