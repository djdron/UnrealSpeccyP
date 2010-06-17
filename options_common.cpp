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
#include "options_common.h"

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
		eOptionInt::Change(J_FIRST, J_LAST, next);
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
		eOptionInt::Change(S_FIRST, S_LAST, next);
	}
} op_sound;

static struct eOptionVolume : public xOptions::eOptionInt
{
	eOptionVolume() { Set(V_50); }
	virtual const char* Name() const { return "volume"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "mute", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(V_FIRST, V_LAST, next);
	}
} op_volume;

static struct eOptionPause : public xOptions::eOptionBool
{
	eOptionPause() { storeable = false; }
	virtual const char* Name() const { return "pause"; }
	virtual void Change(bool next = true)
	{
		eOptionBool::Change();
		Handler()->VideoPaused(self);
	}
} op_pause;

#endif//USE_UI

static struct eOptionDrive : public xOptions::eOptionInt
{
	eOptionDrive() { storeable = false; }
	virtual const char* Name() const { return "drive"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "A", "B", "C", "D", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(D_FIRST, D_LAST, next);
	}
} op_drive;

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
} op_quit;

}
//namespace xPlatform
