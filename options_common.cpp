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
#include "ui/ui.h"
#include "options_common.h"

OPTION_USING(eOptionB, op_open_file);
OPTION_USING(eOptionB, op_ay);

namespace xPlatform
{

static struct eOptionLastFile : public xOptions::eOptionString
{
	eOptionLastFile() { customizable = false; }
	const char* Folder()
	{
		static char lf[xIo::MAX_PATH_LEN];
		strcpy(lf, value);
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
	virtual const char* Name() const { return "last file"; }
} op_last_file;
DECLARE_OPTION(eOptionString, op_last_file);

const char* OpLastFolder() { return op_last_file.Folder(); }

#if defined(USE_UI) || defined(USE_OPTIONS_COMMON)

struct eOptionState : public xOptions::eOptionB
{
	eOptionState() { storeable = false; }
	const char* SnapshotName() const
	{
		static char name[xIo::MAX_PATH_LEN];
		strcpy(name, op_last_file);
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
} op_save_state;
DECLARE_OPTION(eOptionB, op_save_state);

static struct eOptionLoadState : public eOptionState
{
	virtual const char* Name() const { return "load state"; }
	virtual void Change(bool next = true)
	{
		const char* name = SnapshotName();
		if(name)
			Handler()->OnOpenFile(name);
	}
} op_load_state;
DECLARE_OPTION(eOptionB, op_load_state);

OPTION_USING(eOptionBool, op_tape_fast);
static struct eOptionTape : public xOptions::eRootOption<xOptions::eOptionInt>
{
	typedef xOptions::eRootOption<xOptions::eOptionInt> eInherited;
	eOptionTape() { storeable = false; }
	virtual const char* Name() const { return "tape"; }
	virtual int Order() const { return 5; }
protected:
	virtual const char** Values() const
	{
		static const char* values[] = { "n/a", "stop", "start", NULL };
		return values;
	}
	virtual void OnOption()
	{
		if(changed)
		{
			switch(Handler()->OnAction(A_TAPE_TOGGLE))
			{
			case AR_TAPE_NOT_INSERTED:	Set(0);	break;
			case AR_TAPE_STOPPED:		Set(1);	break;
			case AR_TAPE_STARTED:		Set(2);	break;
			default: break;
			}
		}
		Option(OPTION_GET(op_tape_fast));
	}
} op_tape;

static struct eOptionSoundSource : public xOptions::eOptionInt
{
	eOptionSoundSource() { Set(S_AY); }
	virtual const char* Name() const { return "source"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "beeper", "ay", "tape", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(S_LAST, next);
	}
} op_sound_source;
DECLARE_OPTION(eOptionInt, op_sound_source);

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
		eOptionInt::Change(V_LAST, next);
	}
} op_volume;
DECLARE_OPTION(eOptionInt, op_volume);

static struct eOptionPause : public xOptions::eRootOption<xOptions::eOptionBool>
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

#else//USE_UI || USE_OPTIONS_COMMON

DECLARE_OPTION_VOID(eOptionB, op_save_state);
DECLARE_OPTION_VOID(eOptionB, op_load_state);
DECLARE_OPTION_VOID(eOptionInt, op_sound_source);
DECLARE_OPTION_VOID(eOptionInt, op_volume);

#endif//USE_UI || USE_OPTIONS_COMMON

static struct eOptionDrive : public xOptions::eOptionInt
{
	eOptionDrive() { storeable = false; Set(D_A); }
	virtual const char* Name() const { return "drive"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "A", "B", "C", "D", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(D_LAST, next);
	}
} op_drive;
DECLARE_OPTION(eOptionInt, op_drive);

OPTION_USING(eOptionBool, op_auto_play_image);
static struct eOptionImage : public xOptions::eRootOption<xOptions::eOptionB>
{
	virtual const char* Name() const { return "image"; }
protected:
	virtual void OnOption()
	{
		Option(op_last_file);
		Option(OPTION_GET(op_open_file));
		Option(op_drive);
		Option(OPTION_GET(op_auto_play_image));
		Option(op_save_state);
		Option(op_load_state);
	}
} op_image;

static struct eOptionJoy : public xOptions::eRootOption<xOptions::eOptionInt>
{
	eOptionJoy() { Set(J_KEMPSTON); }
	dword KeyFlags()
	{
		switch(value)
		{
		case J_KEMPSTON:	return KF_KEMPSTON;
		case J_CURSOR:		return KF_CURSOR;
		case J_QAOP:		return KF_QAOP;
		case J_SINCLAIR2:	return KF_SINCLAIR2;
		}
		return KF_QAOP;
	}
	virtual const char* Name() const { return "joystick"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "kempston", "cursor", "qaop", "sinclair2", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(J_LAST, next);
	}
	virtual int Order() const { return 10; }
} op_joy;
DECLARE_OPTION(eOptionInt, op_joy);

dword OpJoyKeyFlags() { return op_joy.KeyFlags(); }

static struct eOptionSound : public xOptions::eRootOption<xOptions::eOptionB>
{
	virtual const char* Name() const { return "sound"; }
	virtual int Order() const { return 25; }
protected:
	virtual void OnOption()
	{
		Option(op_sound_source);
		Option(op_volume);
		Option(OPTION_GET(op_ay));
	}
} op_sound;

static struct eOptionReset : public xOptions::eRootOption<xOptions::eOptionB>
{
	eOptionReset() { storeable = false; }
	virtual const char* Name() const { return "reset"; }
	virtual void Change(bool next = true) { Handler()->OnAction(A_RESET); }
	virtual int Order() const { return 80; }
} op_reset;

static struct eOptionQuit : public xOptions::eRootOption<xOptions::eOptionBool>
{
	eOptionQuit() { storeable = false; }
	virtual const char* Name() const { return "quit"; }
	virtual int Order() const { return 100; }
	virtual const char** Values() const { return NULL; }
} op_quit;
DECLARE_OPTION(eOptionBool, op_quit);

}
//namespace xPlatform
