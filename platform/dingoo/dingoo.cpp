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

#include "../platform.h"
#include "../io.h"
#include "../../ui/ui.h"
#include "../../tools/options.h"
#include "../../options_common.h"

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

namespace xPlatform
{

enum eRaySync { RS_FIRST, RS_OFF = RS_FIRST, RS_ON, RS_MIRROR_H, RS_MIRROR_V, RS_MIRROR_HV, RS_LAST };

static struct eOptionRaySync : public xOptions::eOptionInt
{
	virtual const char* Name() const { return "ray sync"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "on", "mirror_h", "mirror_v", "mirror_hv", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(RS_FIRST, RS_LAST, next);
	}
} op_ray_sync;

class eTimer
{
	enum { CHANNEL = 3, REFRESH_LATENCY = CFG_EXTAL / 4 / 50 };
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

class eVideo
{
public:
	eVideo() : frame(NULL), frame_back(NULL), ray_sync(false)
	{
		Set(0x2b, 0x000d);	//max refresh rate
		Set(0x22);			//write to GRAM
		frame = (word*)_lcd_get_frame();
		frame_back = (word*)g_pGameDecodeBuf;

		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? BRIGHTNESS + BRIGHT_INTENSITY : BRIGHTNESS;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			colors888[c] = xUi::eRGBAColor(r, g, b).rgba;
			colors565[c] = BGR565(r, g, b);
		}
	}
	~eVideo() 
	{
		if(ray_sync)
		{
			int mirr = op_ray_sync;
			mirr = mirr ? mirr - 1 : 0;
			Set(0x03, 0x1048|(~mirr&3 << 4)); //entry mode restore
		}
		Set(0x2b, 0x000d);	//default refresh rate
		Set(0x22);			//write to GRAM
	}
	void Flip();
	void Update();
protected:
	void Set(dword cmd, int data = -1);
	word* FrameBack() { return frame_back; }
	enum { BRIGHTNESS = 190, BRIGHT_INTENSITY = 65 };
	inline dword BGR565(byte r, byte g, byte b) const { return (((r&~7) << 8)|((g&~3) << 3)|(b >> 3)); }
protected:
	word*	frame;
	word*	frame_back;
	word	colors565[16];
	dword	colors888[16];
	bool	ray_sync;
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
	byte* src = (byte*)Handler()->VideoData();
	dword* src_ui = (dword*)Handler()->VideoDataUI();
	word* dst = video.FrameBack();
	int mirr = op_ray_sync;
	if(mirr && !ray_sync)
	{
		ray_sync = true;
		Set(0x03, 0x1070);	//entry mode default
		Set(0x22);			//write to GRAM
	}
	mirr = mirr ? mirr - 1 : 0;
	bool mirr_h = mirr & 1;
	bool mirr_v = mirr & 2;
	int offs_base = !ray_sync ? 0 : mirr_h ? 319 : 0;
	int ext_step = !ray_sync ? 320 : mirr_h ? -1 : 1;
	int int_step = !ray_sync ? 1 : mirr_v ? -320 : 320;
	int ext_end = !ray_sync ? 320*240 : mirr_h ? -1 : 320;
	int int_end = !ray_sync ? 320 : mirr_v ? -320 : 320*240;
	offs_base += mirr_v ? 320*239 : 0;
	ext_end += mirr_v ? 320*239 : 0;
	int_end += mirr_h ? 319 : 0;
	if(src_ui)
	{
		for(; offs_base != ext_end; offs_base += ext_step, int_end += ext_step)
		{
			for(int offs = offs_base; offs != int_end; offs += int_step)
			{
				xUi::eRGBAColor c_ui = src_ui[offs];
				xUi::eRGBAColor c = colors888[src[offs]];
				*dst++ = BGR565((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
		}
	}
	else
	{
		for(; offs_base != ext_end; offs_base += ext_step, int_end += ext_step)
		{
			for(int offs = offs_base; offs != int_end; offs += int_step)
			{
				*dst++ = colors565[src[offs]];
			}
		}
	}
}


static xOptions::eOption<int>* op_sound = NULL;
static xOptions::eOption<int>* op_volume = NULL;

static int OpVolume() { return op_volume ? *op_volume : (int)V_100; }
static int OpSound() { return op_sound ? *op_sound : (int)S_AY; }

class eAudio
{
public:
	eAudio() : volume(0)
	{
		op_sound = xOptions::eOption<int>::Find("sound");
		op_volume = xOptions::eOption<int>::Find("volume");
		SetVolume(volume);
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
protected:
	void SetVolume(int v)
	{
		waveout_set_volume(v);
	}
protected:
	void* handle;
	int source;
	int volume;
}audio;

void eAudio::Update()
{
	if(OpVolume() != volume)
	{
		volume = OpVolume();
		SetVolume(volume * 3);
	}
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		bool ui_enabled = Handler()->VideoDataUI();
		if(i == OpSound() && !ui_enabled && !Handler()->FullSpeed())
		{
			waveout_write(handle, Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
}

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
	struct eKeyStatus
	{
		dword pressed;
		dword released;
		dword status;
	};
protected:
	eKeyStatus Status() const
	{
		eKeyStatus ks;
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
}keys;

void eKeys::Update()
{
	if(Pressed(K_BUTTON_SELECT) && Pressed(K_BUTTON_START))
	{
		OpQuit(true);
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
				SAFE_CALL(op_sound)->Change();
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
}


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
	while(!OpQuit() && _sys_judge_event(NULL) >= 0)
	{
		if(!Handler()->FullSpeed())
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
