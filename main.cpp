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

#include "std.h"
#include "platform/platform.h"
#include "speccy.h"
#include "devices/ula.h"
#include "devices/input/keyboard.h"
#include "devices/input/kempston_joy.h"
#include "devices/input/kempston_mouse.h"
#include "devices/input/tape.h"
#include "devices/sound/ay.h"
#include "devices/sound/beeper.h"
#include "devices/fdd/wd1793.h"
#include "z80/z80.h"
#include "snapshot.h"
#include "platform/io.h"
#include "tools/profiler.h"

#include "ui/desktop.h"
#include "platform/custom_ui/main.h"

namespace xPlatform
{

static struct eSpeccyHandler : public eHandler
{
	eSpeccyHandler() : speccy(NULL), video_paused(0), drive_for_open(0)
		, joystick(J_KEMPSTON), sound(S_AY), volume(V_100), quit(false) {}
	virtual ~eSpeccyHandler() { assert(!speccy); }
	virtual void OnInit()
	{
		assert(!speccy);
		speccy = new eSpeccy;
#ifdef USE_UI
		ui_desktop = new xUi::eDesktop;
		ui_desktop->Insert(new xUi::eMainDialog);
#endif//USE_UI
		sound_dev[0] = speccy->Device<eBeeper>();
		sound_dev[1] = speccy->Device<eAY>();
		sound_dev[2] = speccy->Device<eTape>();
	}
	virtual void OnDone()
	{
		SAFE_DELETE(speccy);
#ifdef USE_UI
		SAFE_DELETE(ui_desktop);
#endif//USE_UI
		PROFILER_DUMP;
	}
	virtual void OnLoop()
	{
		if(FullSpeed() || !video_paused)
			speccy->Update();
#ifdef USE_UI
		ui_desktop->Update();
#endif//USE_UI
	}
	virtual void* VideoData() { return speccy->Device<eUla>()->Screen(); }
	virtual void* VideoDataUI()
	{
#ifdef USE_UI
		return ui_desktop->VideoData();
#else//USE_UI
		return NULL;
#endif//USE_UI
	}
	virtual const char* WindowCaption() { return "Unreal Speccy Portable"; }
	virtual void OnKey(char key, dword flags)
	{
		bool down = (flags&KF_DOWN) != 0;
		bool shift = (flags&KF_SHIFT) != 0;
		bool ctrl = (flags&KF_CTRL) != 0;
		bool alt = (flags&KF_ALT) != 0;

#ifdef USE_UI
		if(!(flags&KF_UI_SENDER))
		{
			ui_desktop->OnKey(key, flags);
			if(ui_desktop->Focused())
				return;
		}
#endif//USE_UI

		if(flags&KF_KEMPSTON)
			speccy->Device<eKempstonJoy>()->OnKey(key, down);
		if(flags&KF_CURSOR)
		{
			switch(key)
			{
			case 'l' : key = '5'; shift = down; break;
			case 'r' : key = '8'; shift = down; break;
			case 'u' : key = '7'; shift = down; break;
			case 'd' : key = '6'; shift = down; break;
			case 'f' : key = '0'; shift = false; break;
			}
		}
		else if(flags&KF_QAOP)
		{
			switch(key)
			{
			case 'l' : key = 'O'; break;
			case 'r' : key = 'P'; break;
			case 'u' : key = 'Q'; break;
			case 'd' : key = 'A'; break;
			case 'f' : key = ' '; break;
			}
		}
		else if(flags&KF_SINCLAIR2)
		{
			switch(key)
			{
			case 'l' : key = '6'; break;
			case 'r' : key = '7'; break;
			case 'u' : key = '9'; break;
			case 'd' : key = '8'; break;
			case 'f' : key = '0'; break;
			}
		}
		speccy->Device<eKeyboard>()->OnKey(key, down, shift, ctrl, alt);
	}
	virtual void OnMouse(eMouseAction action, byte a, byte b)
	{
		switch(action)
		{
		case MA_MOVE: 	speccy->Device<eKempstonMouse>()->OnMouseMove(a, b); 	break;
		case MA_BUTTON:	speccy->Device<eKempstonMouse>()->OnMouseButton(a, b != 0);	break;
		default: break;
		}
	}

	virtual bool OnOpenFile(const char* name)
	{
		int l = strlen(name);
		if(l > 3)
		{
			const char* n = name + l - 4;
			if(!strcmp(n, ".trd") || !strcmp(n, ".TRD") || !strcmp(n, ".scl") || !strcmp(n, ".SCL"))
			{
				return speccy->Device<eWD1793>()->Open(name, drive_for_open);
			}
			else if(!strcmp(n, ".sna") || !strcmp(n, ".SNA") ||
					!strcmp(n, ".z80") || !strcmp(n, ".Z80"))
			{
				return xSnapshot::Load(speccy, name);
			}
			else if(!strcmp(n, ".tap") || !strcmp(n, ".TAP") ||
					!strcmp(n, ".csw") || !strcmp(n, ".CSW") ||
					!strcmp(n, ".tzx") || !strcmp(n, ".TZX")
				)
			{
				return speccy->Device<eTape>()->Open(name);
			}
		}
		return false;
	}
	virtual bool OnSaveFile(const char* name)
	{
		int l = strlen(name);
		if(l > 3)
		{
			const char* n = name + l - 4;
			if(!strcmp(n, ".sna") || !strcmp(n, ".SNA"))
			{
				return xSnapshot::Store(speccy, name);
			}
		}
		return false;
	}
	virtual eActionResult OnAction(eAction action)
	{
		switch(action)
		{
		case A_RESET:
			speccy->Reset();
			return AR_RESET_OK;
		case A_TAPE_TOGGLE:
			{
				eTape* tape = speccy->Device<eTape>();
				if(!tape->Inserted())
					return AR_TAPE_NOT_INSERTED;
				if(!tape->Started())
					tape->Start();
				else
					tape->Stop();
				return tape->Started() ? AR_TAPE_STARTED : AR_TAPE_STOPPED;
			}
		case A_TAPE_FAST_TOGGLE:
			{
				eTape* tape = speccy->Device<eTape>();
				if(!tape->Inserted())
					return AR_TAPE_NOT_INSERTED;
				if(!speccy->CPU()->FastEmul())
					speccy->CPU()->FastEmul(FastTapeEmul);
				else
					speccy->CPU()->FastEmul(NULL);
				return speccy->CPU()->FastEmul() ? AR_TAPE_FAST_SET : AR_TAPE_FAST_RESET;
			}
		case A_DRIVE_NEXT:
			{
				switch(++drive_for_open)
				{
				case 4:	drive_for_open = 0; return AR_DRIVE_A;
				case 1:	return AR_DRIVE_B;
				case 2:	return AR_DRIVE_C;
				case 3:	return AR_DRIVE_D;
				}
				return AR_ERROR;
			}
		case A_JOYSTICK_NEXT:
			if(++joystick == J_LAST)
				joystick = J_FIRST;
			return AR_OK;
		case A_SOUND_NEXT:
			if(++sound == S_LAST)
				sound = S_FIRST;
			return AR_OK;
		case A_VOLUME_NEXT:
			if(++volume == V_LAST)
				volume = V_FIRST;
			return AR_OK;
		case A_QUIT:
			quit = true;
			return AR_OK;
		}
		return AR_ERROR;
	}

	virtual int	AudioSources() { return FullSpeed() ? 0 : SOUND_DEV_COUNT; }
	virtual void* AudioData(int source) { return sound_dev[source]->AudioData(); }
	virtual dword AudioDataReady(int source) { return sound_dev[source]->AudioDataReady(); }
	virtual void AudioDataUse(int source, dword size) { sound_dev[source]->AudioDataUse(size); }
	virtual void VideoPaused(bool paused) {	paused ? ++video_paused : --video_paused; }

	virtual bool TapeInserted() const { return speccy->Device<eTape>()->Inserted(); }
	virtual bool TapeStarted() const { return speccy->Device<eTape>()->Started(); }
	virtual bool FullSpeed() const { return speccy->CPU()->FastEmul(); }
	virtual bool Quit() const { return quit; }

	virtual eJoystick Joystick() const { return (eJoystick)joystick; }
	virtual eSound Sound() const { return (eSound)sound; }
	virtual eVolume Volume() const { return (eVolume)volume; }

	eSpeccy* speccy;
#ifdef USE_UI
	xUi::eDesktop* ui_desktop;
#endif//USE_UI
	int video_paused;
	int drive_for_open;
	int joystick;
	int sound;
	int volume;
	bool quit;

	enum { SOUND_DEV_COUNT = 3 };
	eDeviceSound* sound_dev[SOUND_DEV_COUNT];
} sh;

}
//namespace xPlatform

// see platform-specific files for main() function
