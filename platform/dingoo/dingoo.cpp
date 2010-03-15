#include "../../std.h"
#include "../platform.h"

#ifdef _DINGOO

enum eKeyBit
{
	K_POWER			= 7,
	K_BUTTON_A		= 31,
	K_BUTTON_B		= 21,
	K_BUTTON_X		= 16,	// Gemei x760 triangle
	K_BUTTON_Y      = 6,	// Gemei x760 X
	K_BUTTON_START	= 11,	// Does not exist on the Gemei x760
	K_BUTTON_SELECT	= 10,	// Does not exist on the Gemei x760

	K_TRIGGER_LEFT	= 8,	// Does not exist on the Gemei x760
	K_TRIGGER_RIGHT	= 29,	// Does not exist on the Gemei x760

	K_DPAD_UP		= 20,
	K_DPAD_DOWN		= 27,
	K_DPAD_LEFT		= 28,
	K_DPAD_RIGHT	= 18
};

extern "C"
{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

//entry.a externals
char*	__to_locale_ansi(wchar_t*);
int		_sys_judge_event(void*);
void	_lcd_set_frame();
void*	_lcd_get_frame();
void	__dcache_writeback_all();
FILE*	fsys_fopen(const char*, const char*);
int		fsys_fread(void*, size_t, size_t, FILE*);
int		fsys_fclose(FILE*);
int		fsys_fseek(FILE*, long int, int);
long int fsys_ftell(FILE*);

struct eKeyStatus
{
	dword pressed;
	dword released;
	dword status;
};
void	_kbd_get_status(eKeyStatus* ks);
};

#define fopen	fsys_fopen
#define fread	fsys_fread
#define fclose	fsys_fclose
#define fseek	fsys_fseek
#define ftell	fsys_ftell

word* g_pGameDecodeBuf = NULL;

static bool KeyPressed(eKeyBit key)
{
	eKeyStatus ks;
	_kbd_get_status(&ks);
	bool pressed = ks.status & (1 << key);
	return pressed;
}

namespace xPlatform
{

bool Init(int argc, char* argv[])
{
	return true;
}

static const byte brightness = 200;
static const byte bright_intensity = 55;

void Loop()
{
	while(!KeyPressed(K_BUTTON_SELECT))
	{
		byte* src = (byte*)Handler()->VideoData();
		word* dst = (word*)_lcd_get_frame();
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				byte r, g, b;
				byte c = src[y*320+x];
				byte i = c&8 ? brightness + bright_intensity : brightness;
				b = c&1 ? i : 0;
				r = c&2 ? i : 0;
				g = c&4 ? i : 0;
				*dst++ = ((r << 11)|(g << 6)|b);
			}
		}
		__dcache_writeback_all();
		_lcd_set_frame();
	}
}
void Done()
{
}

}
//namespace xPlatform

int main(int argc, char* argv[])
{
	if(!xPlatform::Init(argc, argv))
		return -1;
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

extern "C" int GameMain(char* respath)
{
	_sys_judge_event(NULL); /* We need this, otherwise it won't work :?? */
	char* tempAnsiPath = __to_locale_ansi((wchar_t*)respath);
	int tempAnsiLen = 0;
	for(; tempAnsiPath[tempAnsiLen] != '\0'; ++tempAnsiLen);
	char* tempPath = new char[tempAnsiLen + 1];
	if(tempPath == NULL)
		return EXIT_FAILURE;
	strcpy(tempPath, __to_locale_ansi((wchar_t*)respath));
	int tempOut = main(1, &tempPath);
	delete[] tempPath;
	return tempOut;
}

#endif//_DINGOO
