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
#include "tools/zlib/unzip.h"
#include "tools/profiler.h"
#include "tools/options.h"

namespace xPlatform
{

struct eFileType : public eList<eFileType>
{
	virtual bool Open(const void* data, size_t data_size) = 0;
	virtual bool Store(const char* name) { return false; }
	virtual const char* Type() = 0;
	static eFileType* Find(const char* type)
	{
		eFileType* t = First();
		for(; t && strcmp(t->Type(), type) != 0; t = t->Next())
		{}
		return t;
	}
};

static struct eSpeccyHandler : public eHandler
{
	eSpeccyHandler() : speccy(NULL), video_paused(0), drive_for_open(0) {}
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
		xOptions::Load();
		xOptions::eOption<bool>* op_mode_48k = xOptions::eOption<bool>::Find("mode 48k");
		if(op_mode_48k && *op_mode_48k)
			speccy->Reset();
	}
	virtual void OnDone()
	{
		xOptions::Store();
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
	void GetFileType(char* type, const char* src)
	{
		int l = strlen(src);
		if(!l)
		{
			*type = '\0';
			return;
		}
		const char* ext = src + l;
		while(ext >= src && *ext != '.')
			--ext;
		++ext;
		while(*ext)
		{
			char c = *ext++;
			if(c >= 'A' && c <= 'Z')
				c += 'a' - 'A';
			*type++ = c;
		}
		*type = '\0';
	}
	bool OnOpenZip(const char* name)
	{
		unzFile h = unzOpen(name);
		if(!h)
			return false;
		bool ok = false;
		if(unzGoToFirstFile(h) == UNZ_OK)
		{
			for(;;)
			{
				unz_file_info fi;
				char n[xIo::MAX_PATH_LEN];
				if(unzGetCurrentFileInfo(h, &fi, n, xIo::MAX_PATH_LEN, NULL, 0, NULL, 0) == UNZ_OK)
				{
					char type[xIo::MAX_PATH_LEN];
					GetFileType(type, n);
					eFileType* t = eFileType::Find(type);
					if(t)
					{
						if(unzOpenCurrentFile(h) == UNZ_OK)
						{
							byte* buf = new byte[fi.uncompressed_size];
							if(unzReadCurrentFile(h, buf, fi.uncompressed_size) == int(fi.uncompressed_size))
							{
								ok = t->Open(buf, fi.uncompressed_size);
							}
							delete[] buf;
							unzCloseCurrentFile(h);
						}
					}
				}
				if(ok)
					break;
				if(unzGoToNextFile(h) == UNZ_END_OF_LIST_OF_FILE)
					break;
			}
		}
		unzClose(h);
		return ok;
	}
	virtual bool OnOpenFile(const char* name)
	{
		char type[xIo::MAX_PATH_LEN];
		GetFileType(type, name);
		if(!strcmp(type, "zip"))
		{
			return OnOpenZip(name);
		}
		eFileType* t = eFileType::Find(type);
		if(!t)
			return false;
		FILE* f = fopen(name, "rb");
		if(!f)
			return false;
		fseek(f, 0, SEEK_END);
		size_t size = ftell(f);
		fseek(f, 0, SEEK_SET);
		byte* buf = new byte[size];
		size_t r = fread(buf, 1, size, f);
		fclose(f);
		if(r != size)
		{
			delete[] buf;
			return false;
		}
		bool ok = t->Open(buf, size);
		delete[] buf;
		return ok;
	}
	virtual bool OnSaveFile(const char* name)
	{
		char type[xIo::MAX_PATH_LEN];
		GetFileType(type, name);
		eFileType* t = eFileType::Find(type);
		if(!t)
			return false;
		return t->Store(name);
	}
	virtual eActionResult OnAction(eAction action)
	{
		switch(action)
		{
		case A_RESET:
			speccy->Reset();
			return AR_OK;
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
		}
		return AR_ERROR;
	}

	virtual int	AudioSources() { return FullSpeed() ? 0 : SOUND_DEV_COUNT; }
	virtual void* AudioData(int source) { return sound_dev[source]->AudioData(); }
	virtual dword AudioDataReady(int source) { return sound_dev[source]->AudioDataReady(); }
	virtual void AudioDataUse(int source, dword size) { sound_dev[source]->AudioDataUse(size); }
	virtual void VideoPaused(bool paused) {	paused ? ++video_paused : --video_paused; }

	virtual bool FullSpeed() const { return speccy->CPU()->FastEmul(); }
	eSpeccy* speccy;
#ifdef USE_UI
	xUi::eDesktop* ui_desktop;
#endif//USE_UI
	int video_paused;
	int drive_for_open;

	enum { SOUND_DEV_COUNT = 3 };
	eDeviceSound* sound_dev[SOUND_DEV_COUNT];
} sh;

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
		if(sh.speccy->Device<eRam>()->Mode48k() != self)
		{
			sh.speccy->Device<eRom>()->Mode48k(self);
			sh.speccy->Device<eRam>()->Mode48k(self);
			sh.speccy->Device<eUla>()->Mode48k(self);
			sh.speccy->Devices().Init();
		}
	}
} op_48k;

static struct eFileTypeZ80 : public eFileType
{
	virtual bool Open(const void* data, size_t data_size)
	{
		return xSnapshot::Load(sh.speccy, Type(), data, data_size);
	}
	virtual const char* Type() { return "z80"; }
} ft_z80;
static struct eFileTypeSNA : public eFileTypeZ80
{
	virtual bool Store(const char* name)
	{
		return xSnapshot::Store(sh.speccy, name);
	}
	virtual const char* Type() { return "sna"; }
} ft_sna;
static struct eFileTypeTRD : public eFileType
{
	virtual bool Open(const void* data, size_t data_size)
	{
		return sh.speccy->Device<eWD1793>()->Open(Type(), sh.drive_for_open, data, data_size);
	}
	virtual const char* Type() { return "trd"; }
} ft_trd;
static struct eFileTypeSCL : public eFileTypeTRD
{
	virtual const char* Type() { return "scl"; }
} ft_scl;

static struct eFileTypeTAP : public eFileType
{
	virtual bool Open(const void* data, size_t data_size)
	{
		return sh.speccy->Device<eTape>()->Open(Type(), data, data_size);
	}
	virtual const char* Type() { return "tap"; }
} ft_tap;
static struct eFileTypeCSW : public eFileTypeTAP
{
	virtual const char* Type() { return "csw"; }
} ft_csw;
static struct eFileTypeTZX : public eFileTypeTAP
{
	virtual const char* Type() { return "tzx"; }
} ft_tzx;

}
//namespace xPlatform

// see platform-specific files for main() function
