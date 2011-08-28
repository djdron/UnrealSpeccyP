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
#include "platform/io.h"
#include "tools/options.h"
#include "ui/ui.h"
#include "options_common.h"

namespace xPlatform
{

#if defined(USE_UI) || defined(USE_OPTIONS_COMMON)

struct eOptionState : public xOptions::eOptionB
{
	eOptionState() { storeable = false; }
	const char* SnapshotName() const
	{
		static char name[xIo::MAX_PATH_LEN];
		strcpy(name, OpLastFile());
		int l = strlen(name);
		if(!l || name[l - 1] == '/' || name[l - 1] == '\\')
			return NULL;
		char* e = name + l;
		while(e > name && *e != '.' && *e != '\\' && *e != '/')
			--e;
		if(*e != '.')
			return NULL;
		*e = '\0';
		strcat(name, ".sna");
		return name;
	}
};

static struct eOptionSaveState : public eOptionState
{
	virtual const char* Name() const { return "save state"; }
	virtual void Change(bool next = true)
	{
		const char* name = SnapshotName();
		if(name)
			Handler()->OnSaveFile(name);
	}
	virtual int Order() const { return 1; }
} op_save_state;

static struct eOptionLoadState : public eOptionState
{
	virtual const char* Name() const { return "load state"; }
	virtual void Change(bool next = true)
	{
		const char* name = SnapshotName();
		if(name)
			Handler()->OnOpenFile(name);
	}
	virtual int Order() const { return 2; }
} op_load_state;

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
	virtual int Order() const { return 40; }
} op_tape;

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
	virtual int Order() const { return 20; }
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
	virtual int Order() const { return 30; }
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
	virtual int Order() const { return 70; }
} op_pause;

#endif//USE_UI

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
	virtual int Order() const { return 10; }
} op_joy;

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
	virtual int Order() const { return 60; }
} op_drive;

static struct eOptionReset : public xOptions::eOptionB
{
	eOptionReset() { storeable = false; }
	virtual const char* Name() const { return "reset"; }
	virtual void Change(bool next = true) { Handler()->OnAction(A_RESET); }
	virtual int Order() const { return 80; }
} op_reset;

static struct eOptionQuit : public xOptions::eOptionBool
{
	eOptionQuit() { storeable = false; }
	virtual const char* Name() const { return "quit"; }
	virtual int Order() const { return 100; }
	virtual const char** Values() const { return NULL; }
} op_quit;

static struct eOptionLastFile : public xOptions::eOptionString
{
	eOptionLastFile() { customizable = false; }
	virtual const char* Name() const { return "last file"; }
} op_last_file;

const char* OpLastFile() { return op_last_file; }
const char* OpLastFolder()
{
	static char lf[xIo::MAX_PATH_LEN];
	strcpy(lf, OpLastFile());
	char* n = lf;
	char* n_end = n + strlen(n);
	while(n_end > n && *n_end != '\\' && *n_end != '/')
		--n_end;
	if(*n_end == '\\' || *n_end == '/')
	{
		++n_end;
		*n_end = '\0';
	}
	return lf;
}
void OpLastFile(const char* name) { op_last_file.Set(name); }

bool OpQuit() { return op_quit; }
void OpQuit(bool v) { op_quit.Set(v); }

eDrive OpDrive() { return (eDrive)(int)op_drive; }
void OpDrive(eDrive d) { op_drive.Set(d); }

eJoystick OpJoystick() { return (eJoystick)(int)op_joy; }
void OpJoystick(eJoystick v) { op_joy.Set(v); }
dword OpJoyKeyFlags()
{
	switch(op_joy)
	{
	case J_KEMPSTON:	return KF_KEMPSTON;
	case J_CURSOR:		return KF_CURSOR;
	case J_QAOP:		return KF_QAOP;
	case J_SINCLAIR2:	return KF_SINCLAIR2;
	}
	return KF_QAOP;
}

}
//namespace xPlatform
