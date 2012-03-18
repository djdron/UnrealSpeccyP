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
#include "tools/profiler.h"
#include "tools/options.h"
#include "options_common.h"

#ifdef USE_ZIP
#include "tools/zlib/unzip.h"
#endif//USE_ZIP

namespace xScreenshot
{
bool Store(eSpeccy* speccy, const char* file);
}
//namespace xScreenshot

namespace xPlatform
{

struct eFileType : public eList<eFileType>
{
	virtual bool Open(const void* data, size_t data_size) = 0;
	virtual bool Store(const char* name) { return false; }
	virtual bool AbleOpen() { return true; }
	virtual const char* Type() = 0;
	static eFileType* Find(const char* type)
	{
		eFileType* t = First();
		for(; t && strcmp(t->Type(), type) != 0; t = t->Next())
		{}
		return t;
	}
};

static void GetFileType(char* type, const char* src)
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

static struct eOptionTapeFast : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "fast tape"; }
	virtual int Order() const { return 50; }
} op_tape_fast;

static struct eSpeccyHandler : public eHandler
{
	eSpeccyHandler() : speccy(NULL), video_paused(0) {}
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
	virtual bool FileTypeSupported(const char* name)
	{
		char type[xIo::MAX_PATH_LEN];
		GetFileType(type, name);
		eFileType* t = eFileType::Find(type);
		return t && t->AbleOpen();
	}

	virtual bool OnOpenFile(const char* name, const void* data, size_t data_size)
	{
		OpLastFile(name);
		char type[xIo::MAX_PATH_LEN];
		GetFileType(type, name);
		eFileType* t = eFileType::Find(type);
		if(!t)
			return false;

		if(data && data_size)
			return t->Open(data, data_size);

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
		OpLastFile(name);
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
				{
					if(op_tape_fast)
						speccy->CPU()->FastEmul(FastTapeEmul);
					else
						speccy->CPU()->FastEmul(NULL);
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
	virtual int Order() const { return 65; }
} op_48k;

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
		return sh.speccy->Device<eWD1793>()->Open(Type(), OpDrive(), data, data_size);
	}
	virtual const char* Type() { return "trd"; }
} ft_trd;
static struct eFileTypeSCL : public eFileTypeTRD
{
	virtual const char* Type() { return "scl"; }
} ft_scl;
static struct eFileTypeFDI : public eFileTypeTRD
{
	virtual const char* Type() { return "fdi"; }
} ft_fdi;

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

#ifdef USE_PNG
static struct eFileTypePNG : public eFileType
{
	virtual bool Open(const void* data, size_t data_size) { return false; }
	virtual bool Store(const char* name)
	{
		return xScreenshot::Store(sh.speccy, name);
	}
	virtual const char* Type() { return "png"; }
	virtual bool AbleOpen() { return false; }
} ft_png;
#endif//USE_PNG

#ifdef USE_ZIP
static struct eFileTypeZIP : public eFileType
{
	virtual bool Open(const void* data, size_t data_size);
	virtual const char* Type() { return "zip"; }
} ft_zip;

class eMemoryFile
{
public:
	eMemoryFile(const void* _data, size_t _data_size) : data((const byte*)_data), data_size(_data_size), ptr(NULL) {}
	void Open() { ptr = data; }
	int Close() { ptr = NULL; return 0; }
	size_t Read(void* dst, size_t size)
	{
		if(Pos() + size > data_size)
		{
			size = data_size - Pos();
		}
		memcpy(dst, ptr, size);
		ptr += size;
		return size;
	}
	size_t Pos() const { return ptr - data; }
	enum eSeekMode { S_CUR, S_END, S_SET };
	typedef long int off_t;
	int Seek(off_t offset, eSeekMode mode)
	{
		switch(mode)
		{
		case S_CUR:
			{
				const byte* new_ptr = ptr + offset;
				if(new_ptr < data || new_ptr > data + data_size)
					return -1;
				ptr = new_ptr;
				return 0;
			}
		case S_END:
			if(-offset > (off_t)data_size || offset > 0)
				return -1;
			ptr = data + data_size - offset;
			return 0;
		case S_SET:
			if(offset < 0 || offset > (off_t)data_size)
				return -1;
			ptr = data + offset;
			return 0;
		}
		return -1;
	}
protected:
	const byte* data;
	size_t data_size;
	const byte* ptr;
};
static voidpf ZOpen(voidpf opaque, const void* filename, int mode)
{
	eMemoryFile* f = (eMemoryFile*)filename;
	f->Open();
	return f;
}
static uLong ZRead(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	return f->Read(buf, size);
}
static ZPOS64_T ZTell(voidpf opaque, voidpf stream)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	return f->Pos();
}
static long ZSeek(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	eMemoryFile::eSeekMode sm = eMemoryFile::S_CUR;
    switch(origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR: sm = eMemoryFile::S_CUR; break;
    case ZLIB_FILEFUNC_SEEK_END: sm = eMemoryFile::S_END; break;
    case ZLIB_FILEFUNC_SEEK_SET: sm = eMemoryFile::S_SET; break;
    default: return -1;
    }
    return f->Seek(offset, sm);
}
static int ZClose(voidpf opaque, voidpf stream)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	return f->Close();
}

bool eFileTypeZIP::Open(const void* data, size_t data_size)
{
	eMemoryFile mf(data, data_size);
	zlib_filefunc64_def zfuncs;
	zfuncs.zopen64_file = ZOpen;
	zfuncs.zread_file = ZRead;
	zfuncs.ztell64_file = ZTell;
	zfuncs.zseek64_file = ZSeek;
	zfuncs.zclose_file = ZClose;

	unzFile h = unzOpen2_64(&mf, &zfuncs);
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

#endif//USE_ZIP

}
//namespace xPlatform

// see platform-specific files for main() function
