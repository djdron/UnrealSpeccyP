/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2015 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
#include "speccy.h"
#include "devices/memory.h"
#include "devices/ula.h"
#include "devices/input/keyboard.h"
#include "devices/input/kempston_joy.h"
#include "devices/input/kempston_mouse.h"
#include "devices/input/tape.h"
#include "devices/sound/ay.h"
#include "devices/sound/beeper.h"
#include "devices/fdd/wd1793.h"
#include "z80/z80.h"
#include "snapshot/snapshot.h"
#include "platform/io.h"
#include "ui/ui_desktop.h"
#include "platform/custom_ui/ui_main.h"
#include "tools/profiler.h"
#include "tools/options.h"
#include "tools/io_select.h"
#include "options_common.h"
#include "file_type.h"
#include "snapshot/rzx.h"

#ifdef RG350
int gcw_fullscreen = 1;
#endif//RG350

namespace xPlatform
{

class eMacro
{
public:
	eMacro() : frame(-1) {}
	virtual ~eMacro() {}
	virtual bool Do() = 0;
	virtual bool Update()
	{
		++frame;
		return Do();
	}
protected:
	int frame;
};

static struct eSpeccyHandler : public eHandler, public eRZX::eHandler, public xZ80::eZ80::eHandlerIo
{
	eSpeccyHandler() : speccy(NULL), macro(NULL), replay(NULL), video_paused(0), video_frame(-1), inside_replay_update(false) {}
	virtual ~eSpeccyHandler() { assert(!speccy); }
	virtual void OnInit();
	virtual void OnDone();
	virtual const char* OnLoop();
	virtual void* VideoData() const { return speccy->Device<eUla>()->Screen(); }
	virtual void* VideoDataUI() const
	{
#ifdef USE_UI
		return ui_desktop->VideoData();
#else//USE_UI
		return NULL;
#endif//USE_UI
	}
	virtual const char* WindowCaption() const { return "Unreal Speccy Portable"; }
	virtual void OnKey(char key, dword flags);
	virtual void OnMouse(eMouseAction action, byte a, byte b);
	virtual bool FileTypeSupported(const char* name) const
	{
		const eFileType* t = eFileType::FindByName(name);
		return t && t->AbleOpen();
	}
	virtual bool OnOpenFile(const char* name, const void* data, size_t data_size);
	bool OpenFile(const char* name, const void* data, size_t data_size);
	virtual bool OnSaveFile(const char* name);
	virtual eActionResult OnAction(eAction action);

	virtual int	AudioSources() const { return FullSpeed() ? 0 : SOUND_DEV_COUNT; }
	virtual void* AudioData(int source) const { return sound_dev[source]->AudioData(); }
	virtual dword AudioDataReady(int source) const { return sound_dev[source]->AudioDataReady(); }
	virtual void AudioDataUse(int source, dword size) { sound_dev[source]->AudioDataUse(size); }
	virtual void AudioSetSampleRate(dword sample_rate);
	virtual void VideoPaused(bool paused) {	paused ? ++video_paused : --video_paused; }
	virtual int VideoFrame() const { return video_frame; }

	virtual bool FullSpeed() const { return speccy->CPU()->HandlerStep() != NULL; }

	virtual eSpeccy* Speccy() const { return speccy; }

	void PlayMacro(eMacro* m) { SAFE_DELETE(macro); macro = m; }
	virtual bool RZX_OnOpenSnapshot(const char* name, const void* data, size_t data_size) { return OpenFile(name, data, data_size); }
	virtual byte Z80_IoRead(word port, int tact)
	{
		byte r = 0xff;
		replay->IoRead(&r);
		return r;
	}

	virtual bool GetReplayProgress(dword* frame_current, dword* frames_total, dword* frames_cached)
	{
		if(replay)
			return replay->GetProgress(frame_current, frames_total, frames_cached) == eRZX::E_OK;
		return false;
	}

	const char* RZXErrorDesc(eRZX::eError err) const;
	void Replay(eRZX* r)
	{
		speccy->CPU()->HandlerIo(NULL);
		SAFE_DELETE(replay);
		replay = r;
		if(replay)
			speccy->CPU()->HandlerIo(this);
	}

	eSpeccy* speccy;
#ifdef USE_UI
	xUi::eDesktop* ui_desktop;
#endif//USE_UI
	eMacro* macro;
	eRZX* replay;
	int video_paused;
	int video_frame;
	bool inside_replay_update;

	enum { SOUND_DEV_COUNT = 3 };
	eDeviceSound* sound_dev[SOUND_DEV_COUNT];
} sh;

void eSpeccyHandler::OnInit()
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
	xOptions::Load();
	OnAction(A_RESET);
}
void eSpeccyHandler::OnDone()
{
	xOptions::Store();
	SAFE_DELETE(macro);
	SAFE_DELETE(replay);
	SAFE_DELETE(speccy);
#ifdef USE_UI
	SAFE_DELETE(ui_desktop);
#endif//USE_UI
	PROFILER_DUMP;
}
const char* eSpeccyHandler::OnLoop()
{
	const char* error = NULL;
	if(FullSpeed() || !video_paused)
	{
		if(macro)
		{
			if(!macro->Update())
				SAFE_DELETE(macro);
		}
		if(replay)
		{
			int icount = 0;
			inside_replay_update = true;
			eRZX::eError err = replay->Update(&icount);
			inside_replay_update = false;
			if(err == eRZX::E_OK)
			{
				speccy->Update(&icount);
				err = replay->CheckSync();
			}
			if(err != eRZX::E_OK)
			{
				Replay(NULL);
				error = RZXErrorDesc(err);
			}
		}
		else
			speccy->Update(NULL);
		++video_frame;
	}
#ifdef USE_UI
	ui_desktop->Update();
#endif//USE_UI
	return error;
}
const char* eSpeccyHandler::RZXErrorDesc(eRZX::eError err) const
{
	switch(err)
	{
	case eRZX::E_OK:			return "rzx_ok";
	case eRZX::E_FINISHED:		return "rzx_finished";
	case eRZX::E_SYNC_LOST:		return "rzx_sync_lost";
	case eRZX::E_INVALID:		return "rzx_invalid";
	case eRZX::E_UNSUPPORTED:	return "rzx_unsupported";
	}
	return NULL;
}
void eSpeccyHandler::OnKey(char key, dword flags)
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

	if(flags&KF_CURSORENTER)
	{
		switch(key)
		{
		case 'l' : key = '5'; shift = down; break;
		case 'r' : key = '8'; shift = down; break;
		case 'u' : key = '7'; shift = down; break;
		case 'd' : key = '6'; shift = down; break;
		case 'f' : key = 'e'; shift = false; break;
		}
	}
	else if(flags&KF_CURSOR)
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
	else if(flags&KF_QAOPM)
	{
		switch(key)
		{
		case 'l' : key = 'O'; break;
		case 'r' : key = 'P'; break;
		case 'u' : key = 'Q'; break;
		case 'd' : key = 'A'; break;
		case 'f' : key = 'M'; break;
		}
	}
	else if(flags&KF_QAOPSPACE)
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
	speccy->Device<eKeyboard>()->OnKey(key, down, shift, ctrl, alt);
}
void eSpeccyHandler::OnMouse(eMouseAction action, byte a, byte b)
{
	switch(action)
	{
	case MA_MOVE: 	speccy->Device<eKempstonMouse>()->OnMouseMove(a, b); 	break;
	case MA_BUTTON:	speccy->Device<eKempstonMouse>()->OnMouseButton(a, b != 0);	break;
	default: break;
	}
}
bool eSpeccyHandler::OnOpenFile(const char* name, const void* data, size_t data_size)
{
	OpLastFile(name);
	return OpenFile(name, data, data_size);
}
bool eSpeccyHandler::OpenFile(const char* name, const void* data, size_t data_size)
{
	const eFileType* t = eFileType::FindByName(name);
	if(!t)
		return false;

	if(data && data_size)
		return t->Open(data, data_size);

	return t->Open(name);
}
bool eSpeccyHandler::OnSaveFile(const char* name)
{
	OpLastFile(name);
	const eFileType* t = eFileType::FindByName(name);
	if(!t)
		return false;

	char path[xIo::MAX_PATH_LEN];
	strcpy(path, name);
	int l = strlen(path);
	char* e = path + l;
	while(e > path && *e != '\\' && *e != '/')
		--e;
	*e = 0;
	if(xIo::PathCreate(path))
		return t->Store(name);
	return false;
}

static struct eOptionTapeFast : public xOptions::eOptionBool
{
	eOptionTapeFast() { Set(true); }
	virtual const char* Name() const { return "fast tape"; }
	virtual int Order() const { return 50; }
} op_tape_fast;

static struct eOptionAutoPlayImage : public xOptions::eOptionBool
{
	eOptionAutoPlayImage() { Set(true); }
	virtual const char* Name() const { return "auto play image"; }
	virtual int Order() const { return 55; }
} op_auto_play_image;

static struct eOption48K : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "mode 48k"; }
	virtual void Change(bool next = true)
	{
		eOptionBool::Change();
		Apply();
	}
	virtual void Apply()
	{
		sh.OnAction(A_RESET);
	}
	virtual int Order() const { return 65; }
} op_48k;

static struct eOptionResetToServiceRom : public xOptions::eOptionBool
{
	#ifdef RG350
	virtual const char* Name() const { return "reset to s-rom"; }
	#else
	virtual const char* Name() const { return "reset to service rom"; }
	#endif
	virtual int Order() const { return 79; }
} op_reset_to_service_rom;

#ifdef RG350
static struct eOptionFullscreen : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "fullscreen"; }
	virtual void Change(bool next = true)
	{
		eOptionBool::Change();
		Apply();
	}
	virtual void Apply()
	{
                gcw_fullscreen = !gcw_fullscreen;
	}
	virtual int Order() const { return 75; }
} op_fullscreen;
#endif

eActionResult eSpeccyHandler::OnAction(eAction action)
{
	switch(action)
	{
	case A_RESET:
		if(!inside_replay_update) // can be called from replay->Update()
			SAFE_DELETE(replay);
		SAFE_DELETE(macro);
		speccy->Mode48k(op_48k);
		speccy->Reset();
		if(!speccy->Mode48k())
			speccy->Device<eRom>()->SelectPage(op_reset_to_service_rom ? eRom::ROM_SYS : eRom::ROM_128_1);
		if(inside_replay_update)
			speccy->CPU()->HandlerIo(this);
		return AR_OK;
	case A_TAPE_TOGGLE:
		{
			eTape* tape = speccy->Device<eTape>();
			if(!tape->Inserted())
				return AR_TAPE_NOT_INSERTED;
			if(!tape->Started())
			{
				if(op_tape_fast)
					speccy->CPU()->HandlerStep(fast_tape_emul);
				else
					speccy->CPU()->HandlerStep(NULL);
				tape->Start();
			}
			else
				tape->Stop();
			return tape->Started() ? AR_TAPE_STARTED : AR_TAPE_STOPPED;
		}
	case A_TAPE_QUERY:
		{
			eTape* tape = speccy->Device<eTape>();
			if(!tape->Inserted())
				return AR_TAPE_NOT_INSERTED;
			return tape->Started() ? AR_TAPE_STARTED : AR_TAPE_STOPPED;
		}
	case A_DISK_QUERY:
		{
			eWD1793* wd1793 = speccy->Device<eWD1793>();
			return wd1793->DiskChanged(OpDrive()) ? AR_DISK_CHANGED : AR_DISK_NOT_CHANGED;
		}
	}
	return AR_ERROR;
}

void eSpeccyHandler::AudioSetSampleRate(dword sample_rate)
{
	speccy->Device<eAY>()->SetTimings(SNDR_DEFAULT_SYSTICK_RATE, SNDR_DEFAULT_AY_RATE, sample_rate);
	speccy->Device<eBeeper>()->SetTimings(SNDR_DEFAULT_SYSTICK_RATE, sample_rate);
	speccy->Device<eTape>()->SetTimings(SNDR_DEFAULT_SYSTICK_RATE, sample_rate);
}


static void SetupSoundChip();
static struct eOptionSoundChip : public xOptions::eOptionInt
{
	eOptionSoundChip() { Set(SC_AY); }
	enum eType { SC_FIRST, SC_AY = SC_FIRST, SC_YM, SC_LAST };
	virtual const char* Name() const { return "sound chip"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "ay", "ym", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(SC_FIRST, SC_LAST, next);
		Apply();
	}
	virtual void Apply()
	{
		SetupSoundChip();
	}
	virtual int Order() const { return 24; }
}op_sound_chip;

static struct eOptionAYStereo : public xOptions::eOptionInt
{
	eOptionAYStereo() { Set(AS_ABC); }
	enum eMode { AS_FIRST, AS_ABC = AS_FIRST, AS_ACB, AS_BAC, AS_BCA, AS_CAB, AS_CBA, AS_MONO, AS_LAST };
	virtual const char* Name() const { return "ay stereo"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "abc", "acb", "bac", "bca", "cab", "cba", "mono", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(AS_FIRST, AS_LAST, next);
		Apply();
	}
	virtual void Apply()
	{
		SetupSoundChip();
	}
	virtual int Order() const { return 25; }
}op_ay_stereo;

void SetupSoundChip()
{
	eOptionSoundChip::eType chip = (eOptionSoundChip::eType)(int)op_sound_chip;
	eOptionAYStereo::eMode stereo = (eOptionAYStereo::eMode)(int)op_ay_stereo;
	eAY* ay = sh.speccy->Device<eAY>();
	const SNDCHIP_PANTAB* sndr_pan = SNDR_PAN_MONO;
	switch(stereo)
	{
	case eOptionAYStereo::AS_ABC: sndr_pan = SNDR_PAN_ABC; break;
	case eOptionAYStereo::AS_ACB: sndr_pan = SNDR_PAN_ACB; break;
	case eOptionAYStereo::AS_BAC: sndr_pan = SNDR_PAN_BAC; break;
	case eOptionAYStereo::AS_BCA: sndr_pan = SNDR_PAN_BCA; break;
	case eOptionAYStereo::AS_CAB: sndr_pan = SNDR_PAN_CAB; break;
	case eOptionAYStereo::AS_CBA: sndr_pan = SNDR_PAN_CBA; break;
	case eOptionAYStereo::AS_MONO: sndr_pan = SNDR_PAN_MONO; break;
	case eOptionAYStereo::AS_LAST: break;
	}
	ay->SetChip(chip == eOptionSoundChip::SC_AY ? eAY::CHIP_AY : eAY::CHIP_YM);
	ay->SetVolumes(0x7FFF, chip == eOptionSoundChip::SC_AY ? SNDR_VOL_AY : SNDR_VOL_YM, sndr_pan);
}

static struct eFileTypeRZX : public eFileType
{
	virtual bool Open(const void* data, size_t data_size) const
	{
		eRZX* rzx = new eRZX;
		if(rzx->Open(data, data_size, &sh) == eRZX::E_OK)
		{
			sh.Replay(rzx);
			return true;
		}
		else
		{
			sh.Replay(NULL);
			SAFE_DELETE(rzx);
		}
		return false;
	}
	virtual const char* Type() const { return "rzx"; }
} ft_rzx;

static struct eFileTypeZ80 : public eFileType
{
	virtual bool Open(const void* data, size_t data_size) const
	{
		sh.OnAction(A_RESET);
		return xSnapshot::Load(sh.speccy, Type(), data, data_size);
	}
	virtual const char* Type() const { return "z80"; }
} ft_z80;
static struct eFileTypeSZX : public eFileTypeZ80
{
	virtual const char* Type() const { return "szx"; }
} ft_szx;
static struct eFileTypeSNA : public eFileTypeZ80
{
	virtual bool Store(const char* name) const
	{
		return xSnapshot::Store(sh.speccy, name);
	}
	virtual const char* Type() const { return "sna"; }
} ft_sna;

class eMacroDiskRunBootable : public eMacro
{
	virtual bool Do()
	{
		switch(frame)
		{
		case 70:	sh.OnKey('7', KF_DOWN|KF_SHIFT|KF_UI_SENDER);	break;
		case 72:	sh.OnKey('7', KF_UI_SENDER);					break;
		case 100:	sh.OnKey('e', KF_DOWN|KF_UI_SENDER);			break;
		case 102:	sh.OnKey('e', KF_UI_SENDER);					break;
		case 140:	sh.OnKey('R', KF_DOWN|KF_UI_SENDER);			break;
		case 142:	sh.OnKey('R', KF_UI_SENDER);					break;
		case 160:	sh.OnKey('e', KF_DOWN|KF_UI_SENDER);			break;
		case 162:	sh.OnKey('e', KF_UI_SENDER);
			return false;
		}
		return true;
	}
};

class eMacroDiskRunWithMaxPetrov : public eMacro
{
	virtual bool Do()
	{
		switch(frame)
		{
		case 100:	sh.OnKey('e', KF_DOWN|KF_UI_SENDER);			break;
		case 102:	sh.OnKey('e', KF_UI_SENDER);					break;
		case 200:	sh.OnKey('e', KF_DOWN|KF_UI_SENDER);			break;
		case 202:	sh.OnKey('e', KF_UI_SENDER);
			return false;
		}
		return true;
	}
};

static struct eFileTypeTRD : public eFileType
{
	virtual bool Open(const void* data, size_t data_size) const
	{
		eWD1793* wd = sh.speccy->Device<eWD1793>();
		bool ok = wd->Open(Type(), OpDrive(), data, data_size);
		if(ok && op_auto_play_image)
		{
			sh.OnAction(A_RESET);
			if(sh.speccy->Mode48k())
			{
				sh.speccy->Device<eRom>()->SelectPage(eRom::ROM_DOS);
			}
			else
			{
				if(wd->Bootable(OpDrive()))
				{
					sh.speccy->Device<eRom>()->SelectPage(eRom::ROM_128_1);
					sh.PlayMacro(new eMacroDiskRunBootable);
				}
				else
				{
					sh.speccy->Device<eRom>()->SelectPage(eRom::ROM_SYS);
					sh.PlayMacro(new eMacroDiskRunWithMaxPetrov);
				}
			}
		}
		return ok;
	}
	virtual bool Store(const char* name) const
	{
		FILE* file = fopen(name, "wb");
		if(!file)
			return false;
		eWD1793* wd = sh.speccy->Device<eWD1793>();
		bool ok = wd->Store(Type(), OpDrive(), file);
		fclose(file);
		return ok;
	}
	virtual const char* Type() const { return "trd"; }
} ft_trd;
static struct eFileTypeSCL : public eFileTypeTRD
{
	virtual const char* Type() const { return "scl"; }
} ft_scl;
static struct eFileTypeFDI : public eFileTypeTRD
{
	virtual const char* Type() const { return "fdi"; }
} ft_fdi;
static struct eFileTypeUDI : public eFileTypeTRD
{
	virtual const char* Type() const { return "udi"; }
} ft_udi;
static struct eFileTypeTD0 : public eFileTypeTRD
{
	virtual const char* Type() const { return "td0"; }
} ft_td0;

class eMacroTapeLoad : public eMacro
{
	virtual bool Do()
	{
		switch(frame)
		{
		case 100:
			sh.OnKey('J', KF_DOWN|KF_UI_SENDER);
			break;
		case 102:
			sh.OnKey('J', KF_UI_SENDER);
			sh.OnKey('P', KF_DOWN|KF_ALT|KF_UI_SENDER);
			break;
		case 104:
			sh.OnKey('P', KF_UI_SENDER);
			break;
		case 110:
			sh.OnKey('P', KF_DOWN|KF_ALT|KF_UI_SENDER);
			break;
		case 112:
			sh.OnKey('P', KF_UI_SENDER);
			break;
		case 120:
			sh.OnKey('e', KF_DOWN|KF_UI_SENDER);
			break;
		case 122:
			sh.OnKey('e', KF_UI_SENDER);
			sh.OnAction(A_TAPE_TOGGLE);
			return false;
		}
		return true;
	}
};

static struct eFileTypeTAP : public eFileType
{
	virtual bool Open(const void* data, size_t data_size) const
	{
		bool ok = sh.speccy->Device<eTape>()->Open(Type(), data, data_size);
		if(ok && op_auto_play_image)
		{
			sh.OnAction(A_RESET);
			sh.speccy->Devices().Get<eRom>()->SelectPage(sh.speccy->Devices().Get<eRom>()->ROM_SOS());
			sh.PlayMacro(new eMacroTapeLoad);
		}
		return ok;
	}
	virtual const char* Type() const { return "tap"; }
} ft_tap;
static struct eFileTypeCSW : public eFileTypeTAP
{
	virtual const char* Type() const { return "csw"; }
} ft_csw;
static struct eFileTypeTZX : public eFileTypeTAP
{
	virtual const char* Type() const { return "tzx"; }
} ft_tzx;

}
//namespace xPlatform

// see platform-specific files for main() function
