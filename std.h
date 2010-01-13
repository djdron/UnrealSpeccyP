#define _WIN32_WINNT        0x0500   // mouse wheel since win2k
// #define _WIN32_IE           0x0300   // for property sheet in win95. without this will not start in 9x
#define DIRECTINPUT_VERSION 0x0500   // joystick since dx 5.0 (for NT4, need 3.0)
#define DIRECTSOUND_VERSION 0x0800
#define DIRECTDRAW_VERSION  0x0500
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <urlmon.h>
#include <mshtmhst.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <malloc.h>
#include <conio.h>
#include <math.h>
#include <process.h>

#ifdef _M_IX86
#include <assert.h>
#else
#define assert(x)
#endif
#include <emmintrin.h>

typedef long long			qshort;
typedef unsigned long long	qword;
typedef	unsigned long		dword;
typedef unsigned short		word;
typedef unsigned char		byte;

enum eZeroValue { ZERO };

#define CACHE_LINE 64
//#define CACHE_ALIGNED __declspec(align(CACHE_LINE))
#define CACHE_ALIGNED /*Alone Coder*/
