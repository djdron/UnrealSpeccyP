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

#include "platform/platform.h"
#include "tools/options.h"
#include "ui/ui.h"

namespace xPlatform
{

#ifdef USE_UI

static struct eOptionJoy : public xOptions::eOptionInt
{
	eOptionJoy() { Set(J_KEMPSTON); }
	virtual const char* Name() const { return "joystick"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "kempston", "cursor", "qaop", "sinclair2", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		Set(self + 1);
		if(self == J_LAST)
			Set(J_FIRST);
		Apply();
	}
	virtual void Apply()
	{
		while(self != Handler()->Joystick())
			Handler()->OnAction(A_JOYSTICK_NEXT);
	}
} op_joy;

static struct eOptionTape : public xOptions::eOptionInt
{
	eOptionTape() { storeable = false; }
	virtual const char* Name() const { return "tape"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "n/a", "stop", "start", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		switch(Handler()->OnAction(A_TAPE_TOGGLE))
		{
		case AR_TAPE_NOT_INSERTED:	Set(0);	break;
		case AR_TAPE_STOPPED:		Set(1);	break;
		case AR_TAPE_STARTED:		Set(2);	break;
		default: break;
		}
	}
} op_tape;

static struct eOptionTapeFast : public xOptions::eOptionBool
{
	eOptionTapeFast() { storeable = false; }
	virtual const char* Name() const { return "fast tape"; }
	virtual void Change(bool next = true)
	{
		switch(Handler()->OnAction(A_TAPE_FAST_TOGGLE))
		{
		case AR_TAPE_FAST_SET:
			Set(true);
			break;
		case AR_TAPE_FAST_RESET:
		case AR_TAPE_NOT_INSERTED:
			Set(false);
			break;
		default:
			break;
		}
	}
} op_tape_fast;

static struct eOptionSound : public xOptions::eOptionInt
{
	eOptionSound() { Set(S_AY); }
	virtual const char* Name() const { return "sound"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "beeper", "ay", "tape", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		Set(self + 1);
		if(self == S_LAST)
			Set(S_FIRST);
		Apply();
	}
	virtual void Apply()
	{
		while(self != Handler()->Sound())
			Handler()->OnAction(A_SOUND_NEXT);
	}
} op_sound;

static struct eOptionVolume : public xOptions::eOptionInt
{
	eOptionVolume() { Set(V_100); }
	virtual const char* Name() const { return "volume"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "mute", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		Set(self + 1);
		if(self == V_LAST)
			Set(V_FIRST);
		Apply();
	}
	virtual void Apply()
	{
		while(self != Handler()->Volume())
			Handler()->OnAction(A_VOLUME_NEXT);
	}
} op_volume;

static struct eOptionPause : public xOptions::eOptionBool
{
	eOptionPause() { storeable = false; }
	virtual const char* Name() const { return "pause"; }
	virtual void Change(bool next = true)
	{
		Set(!self);
		Handler()->VideoPaused(self);
	}
} op_pause;

#endif//USE_UI

static struct eOption48K : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "mode 48k"; }
	virtual void Change(bool next = true)
	{
		Set(!self);
		Apply();
	}
	virtual void Apply()
	{
		while(self != Handler()->Mode48k())
			Handler()->OnAction(A_MODE_48K_TOGGLE);
	}
} op_48k;

#ifdef USE_UI

static struct eOptionReset : public xOptions::eOptionB
{
	eOptionReset() { storeable = false; }
	virtual const char* Name() const { return "reset"; }
	virtual void Change(bool next = true) { Handler()->OnAction(A_RESET); }
} op_reset;

static struct eOptionQuit : public xOptions::eOptionB
{
	eOptionQuit() { storeable = false; }
	virtual const char* Name() const { return "quit"; }
	virtual void Change(bool next = true) { Handler()->OnAction(A_QUIT); }
} op_quit;

#endif//USE_UI

}
//namespace xPlatform
