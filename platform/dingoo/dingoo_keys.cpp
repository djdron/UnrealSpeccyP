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

#ifdef _DINGOO

#include "../platform.h"
#include "../../options_common.h"

#include <dingoo/keyboard.h>

namespace xPlatform
{

class eKeys
{
public:
	eKeys() : status(0) {}
	void Update();
protected:
	enum eKeyBit
	{
		K_POWER			= 1 << 7,
		K_BUTTON_A		= 1 << 31,
		K_BUTTON_B		= 1 << 21,
		K_BUTTON_X		= 1 << 16,
		K_BUTTON_Y      = 1 << 6,
		K_BUTTON_START	= 1 << 11,
		K_BUTTON_SELECT	= 1 << 10,
		K_TRIGGER_LEFT	= 1 << 8,
		K_TRIGGER_RIGHT	= 1 << 29,
		K_DPAD_UP		= 1 << 20,
		K_DPAD_DOWN		= 1 << 27,
		K_DPAD_LEFT		= 1 << 28,
		K_DPAD_RIGHT	= 1 << 18
	};
protected:
	KEY_STATUS Status() const
	{
		KEY_STATUS ks;
		_kbd_get_status(&ks);
		return ks;
	}
	bool Pressed(eKeyBit key) const { return Status().status&key; }
	void UpdateKey(eKeyBit key, char zx_key, dword flags = 0)
	{
		bool pressed = Pressed(key);
		if(pressed == (bool)(status&key))
			return;
		status = pressed ? status|key : status&~key;
		flags |= pressed ? KF_DOWN : 0;
		Handler()->OnKey(zx_key, flags);
	}
protected:
	dword status;
};

void eKeys::Update()
{
	if(Pressed(K_BUTTON_SELECT) && Pressed(K_BUTTON_START))
	{
		OPTION_GET(op_quit)->Set(true);
	}
	dword flags = OpJoyKeyFlags();
	bool ui_focused = Handler()->VideoDataUI();
	if(!ui_focused)
	{
		if(Pressed(K_TRIGGER_LEFT))
		{
			Handler()->OnAction(A_RESET);
		}
		static bool audio_next = false;
		if(Pressed(K_TRIGGER_RIGHT))
		{
			if(!audio_next)
			{
				audio_next = true;
				OPTION_GET(op_sound_source)->Change(true);
			}
			return;
		}
		audio_next = false;
	}
	else
	{
		flags |= Pressed(K_TRIGGER_LEFT) ? KF_SHIFT : 0;
		flags |= Pressed(K_TRIGGER_RIGHT) ? KF_ALT : 0;
		if(!(Status().status&~(K_TRIGGER_LEFT|K_TRIGGER_RIGHT)))
		{
			Handler()->OnKey(0, flags);
		}
	}
	UpdateKey(K_DPAD_UP, 'u', flags);
	UpdateKey(K_DPAD_DOWN, 'd', flags);
	UpdateKey(K_DPAD_LEFT, 'l', flags);
	UpdateKey(K_DPAD_RIGHT, 'r', flags);
	UpdateKey(K_BUTTON_A, 'f', flags);

	UpdateKey(K_BUTTON_B, 'e', flags);
	UpdateKey(K_BUTTON_X, '1', flags);
	UpdateKey(K_BUTTON_Y, ' ', flags);

	UpdateKey(K_BUTTON_SELECT, 'm'); //menu dialog
	UpdateKey(K_BUTTON_START, 'k'); //keys dialog
	UpdateKey(K_POWER, 'p'); //profiler
}

void UpdateKeys() {	static eKeys keys; keys.Update(); }

}
//namespace xPlatform

#endif//_DINGOO
