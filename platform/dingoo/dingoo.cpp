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

#include "../../std.h"
#include "../platform.h"
#include "../io.h"
#include "../../ui/ui.h"

//#define __inline__

#ifdef _DINGOO

extern "C"
{

#define CFG_EXTAL	12000000	/* EXTAL freq: 12 MHz */
#include "jz4740.h"

//entry.a externals
char*	__to_locale_ansi(wchar_t*);
void	__dcache_writeback_all();
void*	_lcd_get_frame();
void	_lcd_set_frame();
void	_kbd_get_status(void*);
int		_sys_judge_event(void*);
void*	waveout_open(void*);
int		waveout_write(void*, void* buffer, int count);
int		waveout_close(void*);
int		waveout_set_volume(dword vol);

size_t strlen(const char* src)
{
	int i = 0;
	for(; src[i]; ++i);
	return i;
}

}
//extern "C"

void* g_pGameDecodeBuf = NULL;

class eTimer
{
	enum { CHANNEL = 3, REFRESH_LATENCY = CFG_EXTAL / 4 / 50 }; //in tcu ticks 0xdf7c
public:
	eTimer()
	{
		__tcu_disable_pwm_output(CHANNEL);
		__tcu_mask_half_match_irq(CHANNEL);
		__tcu_mask_full_match_irq(CHANNEL);
		__tcu_select_extalclk(CHANNEL);
		__tcu_select_clk_div4(CHANNEL);
		__tcu_set_count(CHANNEL, 0);
		__tcu_start_counter(CHANNEL);
	}
	~eTimer()
	{
		__tcu_stop_counter(CHANNEL);
	}
	void Wait()
	{
		while(!IsFuture());
		SetFuture(REFRESH_LATENCY);
	}
protected:
	void SetFuture(word ticks)
	{
		REG_TCU_TDFR(CHANNEL) = ticks;
		REG_TCU_TDHR(CHANNEL) = ticks;
		__tcu_clear_full_match_flag(CHANNEL);
	}
	bool IsFuture() { return __tcu_full_match_flag(CHANNEL); }
}timer;

#define RGB565(r, g, b)	(((b&~7) << 8)|((g&~3) << 3)|(r >> 3))

class eVideo
{
public:
	eVideo() : frame(NULL), frame_back(NULL)
	{
		Set(0x03, 0x0030);	//entry mode default
//		Set(0x2b, 0x0008);	//refresh 51hz
		Set(0x20, 0x0000);
		Set(0x21, 0x0000);
		Set(0x22);			//write to GRAM
		frame = (word*)_lcd_get_frame();
		frame_back = (word*)g_pGameDecodeBuf;

		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? BRIGHTNESS + BRIGHT_INTENSITY : BRIGHTNESS;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			colors[c] = RGB565(r, g, b);
		}
	}
	~eVideo() 
	{
		Set(0x03, 0x1048);	//entry mode restore
		Set(0x2b, 0x000d);	//refresh 93hz
		Set(0x20, 0x0000);
		Set(0x21, 0x0000);
		Set(0x22);			//write to GRAM
	}
	void Flip();
	void Update();
protected:
	void Set(dword cmd, int data = -1);
	word* FrameBack() { return frame_back; }
	enum { BRIGHTNESS = 190, BRIGHT_INTENSITY = 65 };
protected:
	word*	frame;
	word*	frame_back;
	word	colors[16];
}video;

void eVideo::Set(dword cmd, int data)
{
	// SLCD related GPIO pins for the Dingoo A320
	const int pin_rs_n = 32*2+19;	// port 2 pin 19: RS# (register select, active low)

	__gpio_clear_pin(pin_rs_n);
	REG_SLCD_DATA = SLCD_DATA_RS_COMMAND|(cmd&0xffff);
	while(REG_SLCD_STATE&SLCD_STATE_BUSY);
	__gpio_set_pin(pin_rs_n);
	if(data == -1)
		return;
	REG_SLCD_DATA = SLCD_DATA_RS_DATA|(data&0xffff);
	while(REG_SLCD_STATE&SLCD_STATE_BUSY);
}
void eVideo::Flip()
{
	word* tmp = frame;
	frame = frame_back;
	frame_back = tmp;

	__dcache_writeback_all();

	while(!(REG_DMAC_DCCSR(0)&DMAC_DCCSR_TT));	// Wait for transfer terminated bit
	REG_SLCD_CTRL = 1;							// Enable DMA on the SLCD
	REG_DMAC_DCCSR(0) = 0;						// Disable DMA channel while configuring
	REG_DMAC_DRSR(0) = DMAC_DRSR_RS_SLCD;		// DMA request source is SLCD
	REG_DMAC_DSAR(0) = (dword)frame&0x1fffffff;	// Set source, target and count
	REG_DMAC_DTAR(0) = SLCD_FIFO&0x1fffffff;
	REG_DMAC_DTCR(0) = (320*240*2)/16;

	// Source address increment, source width 32 bit,
	// destination width 16 bit, data unit size 16 bytes,
	// block transfer mode, no interrupt
	REG_DMAC_DCMD(0) = DMAC_DCMD_SAI|DMAC_DCMD_SWDH_32
		|DMAC_DCMD_DWDH_16|DMAC_DCMD_DS_16BYTE|DMAC_DCMD_TM;

	REG_DMAC_DCCSR(0) |= DMAC_DCCSR_NDES;		// No DMA descriptor used
	REG_DMAC_DCCSR(0) |= DMAC_DCCSR_EN;			// Set enable bit to start DMA
}

void eVideo::Update()
{
	using namespace xPlatform;
	byte* src = (byte*)Handler()->VideoData();
	dword* src2 = (dword*)Handler()->VideoDataUI();
	word* dst = video.FrameBack();
	if(src2)
	{
		for(int offs_base = 320*240; --offs_base >= 320*239; )
		{
			for(int offs = offs_base; offs >= 0; offs -= 320)
			{
				byte c = src[offs];
				byte i = c&8 ? BRIGHTNESS + BRIGHT_INTENSITY : BRIGHTNESS;
				byte b = c&1 ? i : 0;
				byte r = c&2 ? i : 0;
				byte g = c&4 ? i : 0;
				xUi::eRGBAColor c2 = src2[offs];
				*dst++ = RGB565((r >> c2.a) + c2.r, (g >> c2.a) + c2.g, (b >> c2.a) + c2.b);
			}
		}
	}
	else
	{
		for(int offs_base = 320*240; --offs_base >= 320*239; )
		{
			for(int offs = offs_base; offs >= 0; offs -= 320)
			{
				*dst++ = colors[src[offs]];
			}
		}
	}
}

class eAudio
{
public:
	eAudio() : source(1)
	{
		waveout_set_volume(30);
		struct eWaveoutOpen
		{
			dword	frequency;
			word	bits;
			byte	channels;
			byte	volume;
		}wo = { 44100, 16, 2, 30 };
		handle = waveout_open(&wo);
	}
	~eAudio() { waveout_close(handle); }
	void Update();
	void NextSource()
	{
		source = source < xPlatform::Handler()->AudioSources() - 1 ? source + 1 : 0;
	}
protected:
	void* handle;
	int source;
}audio;

void eAudio::Update()
{
	using namespace xPlatform;
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		bool ui_enabled = Handler()->VideoDataUI();
		if(i == source && !ui_enabled)
		{
			waveout_write(handle, Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
	return;
/*
	//mix sound sources
	static word sound_stream[16384*2];
	dword common_size = 0;
	for(int i = 0; i < Handler()->AudioSources(); ++i)
	{
		dword size = Handler()->AudioDataReady(i);
		if(size > common_size)
			common_size = size;
	}
	for(int i = 0; i < Handler()->AudioSources(); ++i)
	{
		dword size = Handler()->AudioDataReady(i);
		void* data = Handler()->AudioData(i);
		if(!i)
		{
			memcpy(sound_stream, (byte*)data, size);
			memset(sound_stream + size, 0, common_size - size);
		}
		else
		{
			word* src = (word*)data;
			word* dst = &sound_stream;
			for(int j = 0; j < size/2; ++j)			{				*dst++ += *src++;			}		}
		Handler()->AudioDataUse(i, size);
	}
	sound.Write(sound_stream, common_size);*/
}

class eKeys
{
public:
	eKeys() {}
	void Update();
	bool Quit() const
	{
		return Pressed(K_BUTTON_SELECT) && Pressed(K_BUTTON_START);
	}
protected:
	enum eKeyBit
	{
		K_POWER			= 7,
		K_BUTTON_A		= 31,
		K_BUTTON_B		= 21,
		K_BUTTON_X		= 16,
		K_BUTTON_Y      = 6,
		K_BUTTON_START	= 11,
		K_BUTTON_SELECT	= 10,
		K_TRIGGER_LEFT	= 8,
		K_TRIGGER_RIGHT	= 29,
		K_DPAD_UP		= 20,
		K_DPAD_DOWN		= 27,
		K_DPAD_LEFT		= 28,
		K_DPAD_RIGHT	= 18
	};
	struct eKeyStatus
	{
		dword pressed;
		dword released;
		dword status;
	};
protected:
	bool Pressed(eKeyBit key) const
	{
		eKeyStatus ks;
		_kbd_get_status(&ks);
		bool pressed = ks.status & (1 << key);
		return pressed;
	}
	void UpdateKey(eKeyBit key, char zx_key, dword flags = 0)
	{
		flags |= Pressed(key) ? xPlatform::KF_DOWN : 0;
		xPlatform::Handler()->OnKey(zx_key, flags);
	}
}keys;

void eKeys::Update()
{
	using namespace xPlatform;
	dword flags = KF_KEMPSTON;
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
				audio.NextSource();
			}
		}
		else
		{
			audio_next = false;
		}
		UpdateKey(K_DPAD_UP, 'Q');
		UpdateKey(K_DPAD_DOWN, 'A');
		UpdateKey(K_DPAD_LEFT, 'O');
		UpdateKey(K_DPAD_RIGHT, 'P');
		UpdateKey(K_BUTTON_A, 'M');
	}
	else
	{
		flags = Pressed(K_TRIGGER_LEFT) ? KF_SHIFT : 0;
		flags |= Pressed(K_TRIGGER_RIGHT) ? KF_ALT : 0;
		if(flags && !Pressed(K_DPAD_UP) && !Pressed(K_DPAD_DOWN)
			&& !Pressed(K_DPAD_LEFT) && !Pressed(K_DPAD_RIGHT)
			&& !Pressed(K_BUTTON_A) && !Pressed(K_BUTTON_B)
			&& !Pressed(K_BUTTON_X) && !Pressed(K_BUTTON_Y))
		{
			UpdateKey(flags&KF_SHIFT ? K_TRIGGER_LEFT : K_TRIGGER_RIGHT
				, flags&KF_SHIFT ? 'c' : 's', flags);
		}
	}
	UpdateKey(K_DPAD_UP, 'u', flags);
	UpdateKey(K_DPAD_DOWN, 'd', flags);
	UpdateKey(K_DPAD_LEFT, 'l', flags);
	UpdateKey(K_DPAD_RIGHT, 'r', flags);
	UpdateKey(K_BUTTON_A, 'f', flags);

	UpdateKey(K_BUTTON_B, 'e', flags);
	UpdateKey(K_BUTTON_X, '0', flags);
	UpdateKey(K_BUTTON_Y, ' ', flags);

	UpdateKey(K_BUTTON_SELECT, '`'); //open dialog
	UpdateKey(K_BUTTON_START, '\\'); //keys dialog

	static bool tape_toggle = false;
	if(Pressed(K_POWER))
	{
		if(!tape_toggle)
		{
			tape_toggle = true;
			Handler()->OnAction(A_TAPE_TOGGLE);
		}
	}
	else
	{
		tape_toggle = false;
	}
}

namespace xPlatform
{

bool Init(const char* res_path)
{
	char buf[1024];
	strcpy(buf, __to_locale_ansi((wchar_t*)res_path));
	int i = strlen(buf);
	for(; --i >= 0; )
	{
		if((buf[i] == '\\') || (buf[i] == '/'))
			break;
	}
	buf[++i] = '\0';
	xIo::SetResourcePath(buf);
	Handler()->OnInit();
	return true;
}
void Done()
{
	Handler()->OnDone();
}

void Loop()
{
	while(!keys.Quit() && _sys_judge_event(NULL) >= 0)
	{
		timer.Wait();
		video.Flip();
		audio.Update();
		keys.Update();
		Handler()->OnLoop();
		video.Update();
	}
}

}
//namespace xPlatform

// C++ support instead of libstdc++
void* operator new(size_t size) { return malloc(size); }
void* operator new[](size_t size) { return malloc(size); }
void operator delete(void* p) { if(p) free(p); }
void operator delete[](void* p) { if(p) free(p); }
extern "C" void __cxa_pure_virtual() {}

typedef void (*_PVFV)();
extern _PVFV __CTOR_LIST__[];
extern _PVFV __CTOR_END__[];
extern _PVFV __DTOR_LIST__[];
extern _PVFV __DTOR_END__[];

static void CrtCallList(const _PVFV* beg, const _PVFV* end)
{
	for(; beg < end; ++beg)
	{
		if(*beg) (**beg)();
	}
}
static void CrtInit()
{
	CrtCallList(__CTOR_LIST__, __CTOR_END__); //global constructors call
}
static void CrtDone()
{
	CrtCallList(__DTOR_LIST__, __DTOR_END__); //global destructors call
}

extern "C" int GameMain(char* res_path)
{
	CrtInit();
	xPlatform::Init(res_path);
	xPlatform::Loop();
	xPlatform::Done();
	CrtDone();
	return 0;
}

#endif//_DINGOO
