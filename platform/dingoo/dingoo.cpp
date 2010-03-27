#include "../../std.h"
#include "../platform.h"
#include "../io.h"

#ifdef _DINGOO

extern "C"
{

#include "jz4740.h"

//entry.a externals
char*	__to_locale_ansi(wchar_t*);
void	__dcache_writeback_all();
void*	_lcd_get_frame();
void	_lcd_set_frame();
void	_kbd_get_status(void*);
int		_sys_judge_event(void*);
dword	GetTickCount();

FILE*	fsys_fopen(const char*, const char*);
int		fsys_fread(void*, size_t, size_t, FILE*);
int		fsys_fclose(FILE*);
int		fsys_fseek(FILE*, dword, int);
dword	fsys_ftell(FILE*);

}
//extern "C"

inline FILE*	fopen(const char* fname, const char* mode) { return fsys_fopen(fname, mode); }
inline size_t	fread(void* dst, size_t size, size_t count, FILE* f) { return fsys_fread(dst, size, count, f); }
inline int		fclose(FILE* f) { return fsys_fclose(f); }
inline int		fseek(FILE* f, long offset, int origin) { return fsys_fseek(f, offset, origin); }
inline long		ftell(FILE* f) { return fsys_ftell(f); }

void* g_pGameDecodeBuf = NULL;

//SLCD related GPIO pins for the Dingoo A320
#define PIN_RS_N	(32*2+19)	/* Port 2 pin 19: RS# (register select, active low) */

static void SlcdSet(dword cmd, int data = -1)
{
	__gpio_clear_pin(PIN_RS_N);
	REG_SLCD_DATA = SLCD_DATA_RS_COMMAND|(cmd&0xffff);
	while(REG_SLCD_STATE&SLCD_STATE_BUSY);
	__gpio_set_pin(PIN_RS_N);

	if(data == -1)
		return;
	REG_SLCD_DATA = SLCD_DATA_RS_DATA|(data&0xffff);
	while(REG_SLCD_STATE&SLCD_STATE_BUSY);
}

static void SlcdInit()
{
	SlcdSet(0x03, 0x0020);	//entry mode default
	SlcdSet(0x2b, 0x0008);	//refresh 51hz
	SlcdSet(0x20, 0x0000);
	SlcdSet(0x21, 0x0000);
	SlcdSet(0x22);			//write to GRAM
}
static void SlcdDone()
{
	SlcdSet(0x03, 0x1048);	//entry mode restore
	SlcdSet(0x2b, 0x000d);	//refresh 93hz
	SlcdSet(0x20, 0x0000);
	SlcdSet(0x21, 0x0000);
	SlcdSet(0x22);			//write to GRAM
}

namespace xPlatform
{

bool Init(const char* res_path)
{
	char buf[1024];
	strcpy(buf, __to_locale_ansi((wchar_t*)res_path));
	int l = 0;
	for(int i = 0; buf[i] != '\0'; ++i)
	{
		if((buf[i] == '\\') || (buf[i] == '/'))
			l = i;
	}
	buf[++l] = '\0';
	xIo::SetResourcePath(buf);
	Handler()->OnInit();
	return true;
}
void Done()
{
	Handler()->OnDone();
}

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

static bool KeyPressed(dword key)
{
	eKeyStatus ks;
	_kbd_get_status(&ks);
	bool pressed = ks.status & (1 << key);
	return pressed;
}

static void UpdateKey(eKeyBit key, char zx_key)
{
	dword flags = KeyPressed(key) ? KF_DOWN : 0;
	Handler()->OnKey(zx_key, flags);
}

static void UpdateKeys()
{
	if(KeyPressed(K_BUTTON_SELECT))
	{
		Handler()->OnAction(A_RESET);
	}
	UpdateKey(K_DPAD_UP, 'Q');
	UpdateKey(K_DPAD_DOWN, 'A');
	UpdateKey(K_DPAD_LEFT, 'O');
	UpdateKey(K_DPAD_RIGHT, 'P');

	UpdateKey(K_BUTTON_A, 'M');
	UpdateKey(K_BUTTON_B, 'e');
	UpdateKey(K_BUTTON_X, '0');
	UpdateKey(K_BUTTON_Y, ' ');

	UpdateKey(K_TRIGGER_LEFT, 'c');
	UpdateKey(K_TRIGGER_RIGHT, 's');
}

#define RGB565(r, g, b)	(((b&~7) << 8)|((g&~3) << 3)|(r >> 3))

void Loop()
{
	static const byte brightness = 190;
	static const byte bright_intensity = 65;
	static const dword refresh_latency = 960000 / 51;

	dword last_tick = GetTickCount();
	while(!(KeyPressed(K_BUTTON_SELECT) && KeyPressed(K_BUTTON_START)))
	{
		for(dword passed = 0; passed < refresh_latency; )
		{
			passed = GetTickCount() - last_tick;
		}
		last_tick += refresh_latency;
		_lcd_set_frame();
		_sys_judge_event(NULL);
		UpdateKeys();
		Handler()->OnLoop();
		byte* src = (byte*)Handler()->VideoData();
		word* dst = (word*)_lcd_get_frame();
		for(int x = 320; --x >= 0; )
		{
			for(int y = 0; y < 240; ++y)
			{
				byte r, g, b;
				byte c = src[y*320+x];
				byte i = c&8 ? brightness + bright_intensity : brightness;
				b = c&1 ? i : 0;
				r = c&2 ? i : 0;
				g = c&4 ? i : 0;
				*dst++ = RGB565(r, g, b);
			}
		}
		__dcache_writeback_all();
	}
}

}
//namespace xPlatform

// C++ support instead of libstdc++
void* operator new(size_t size) { return malloc(size); }
void* operator new[](size_t size) { return malloc(size); }
void operator delete(void* p) { if(p) free(p); }
void operator delete[](void* p) { if(p) free(p); }

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
	SlcdInit();
	xPlatform::Init(res_path);

	xPlatform::Handler()->OnOpenFile(xIo::ResourcePath("images/vibrate.sna"));
//	xPlatform::Handler()->OnOpenFile(xIo::ResourcePath("images/disk2.trd"));

	xPlatform::Loop();
	xPlatform::Done();
	SlcdDone();
	CrtDone();
	return 0;
}

#endif//_DINGOO
