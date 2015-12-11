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

#ifdef _WIN_MOBILE

#include <windows.h>
#include <aygshell.h>
#pragma comment(lib, "aygshell.lib") 

#include "../../tools/options.h"
#include "../../options_common.h"
#include "../io.h"

namespace xPlatform
{

void OnLoopSound();

BOOL InitInstance(HINSTANCE, int);

static wchar_t resource_path[1024];
static bool handler_inited = false;

bool Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// Perform application initialization:
	if(!InitInstance(hInstance, nCmdShow)) 
	{
		return false;
	}
	int l = GetModuleFileName(NULL, resource_path, 1024);
	for(; --l >= 0 && resource_path[l] != '\\'; )
	{
	}
	resource_path[++l] = '\0';
	char buf[1024];
	l = WideCharToMultiByte(CP_ACP, 0, resource_path, -1, buf, 1024, NULL, NULL);
	buf[l] = '\0';
	xIo::SetResourcePath(buf);
	Handler()->OnInit();
	handler_inited = true;
	return true;
}
void Done()
{
	Handler()->OnDone();
	handler_inited = false;
}

// Global Variables:
HINSTANCE			g_hInst = NULL;				// current instance
HWND				g_hWndMenuBar = NULL;		// menu bar handle

void Loop()
{
//	HACCEL hAccelTable = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDC_TEST));
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

#define MAX_LOADSTRING 100

// Forward declarations of functions included in this code module:
ATOM			MyRegisterClass(HINSTANCE, LPTSTR);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
//INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	TCHAR* szTitle = L"unreal_speccy_portable";		// title bar text
	TCHAR* szWindowClass = L"unreal_speccy_portable";	// main window class name

	g_hInst = hInstance; // Store instance handle in our global variable

	// SHInitExtraControls should be called once during your application's initialization to initialize any
	// of the device specific controls such as CAPEDIT and SIPPREF.
	SHInitExtraControls();

// 	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING); 
// 	LoadString(hInstance, IDC_TEST, szWindowClass, MAX_LOADSTRING);

	//If it is already running, then focus on the window, and exit
	hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		// set focus to foremost child window
		// The "| 0x00000001" is used to bring any owned windows to the foreground and
		// activate them.
		SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
		return 0;
	} 

	ImmDisableIME(0); // disable IME for correct WM_KEYDOWN/WM_KEYUP handle

	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = g_hInst;
	wc.hIcon         = NULL;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEST));
	wc.hCursor       = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	if(!RegisterClass(&wc))
	{
		return FALSE;
	}

	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	// When the main window is created using CW_USEDEFAULT the height of the menubar (if one
	// is created is not taken into account). So we resize the window after creating it
	// if a menubar is present
	if (g_hWndMenuBar)
	{
		RECT rc;
		RECT rcMenuBar;

		GetWindowRect(hWnd, &rc);
		GetWindowRect(g_hWndMenuBar, &rcMenuBar);
		rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);

		MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

#define RGBX(r, g, b)	(((r << 8)&0xf800)|((g << 3)&0x07e0)|(b >> 3))

static void TranslateKey(int vk_key, char& key, dword& flags)
{
	if(vk_key >= '0' && vk_key <= '9')
		key = vk_key;
	else if(vk_key >= 'A' && vk_key <= 'Z')
		key = vk_key;
	switch(vk_key)
	{
	case VK_SHIFT:		key = 'c';	break;
	case VK_MENU:		key = 's';	break;
	case VK_RETURN:		key = 'e';	break;
	case VK_SPACE:		key = ' ';	break;
	case VK_TAB:
		key = '\0';
		flags |= KF_ALT;
		flags |= KF_SHIFT;
		break;
	case VK_BACK:
		key = '0';
		flags |= KF_SHIFT;
		break;
	case VK_LEFT:		key = 'l';	break;
	case VK_RIGHT:		key = 'r';	break;
	case VK_UP:			key = 'u';	break;
	case VK_DOWN:		key = 'd';	break;
	case VK_CONTROL:	key = 'f';	flags &= ~KF_CTRL; break;
	case 0xDE://VK_OEM_7:
		if(flags&KF_SHIFT)
		{
			key = 'P';
			flags |= KF_ALT;
			flags &= ~KF_SHIFT;
		}
		break;
	case 0xBC://VK_OEM_COMMA:
		key = 'N';
		flags |= KF_ALT;
		break;
	case 0xBE://VK_OEM_PERIOD:
		key = 'M';
		flags |= KF_ALT;
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SHACTIVATEINFO s_sai;
	static HDC hdc_mem = NULL;
	static HBITMAP bmp_mem = NULL;
	static word* tex = NULL;
	static HMENU main_menu = NULL;
	static HMENU joy_menu = NULL;

	enum eTimerId { TM_UPDATE = 1 };
	enum eCommandId
	{
		IDM_EXIT = 40000,
		IDM_OPEN_FILE,
		IDM_RESET,
		IDM_TAPE_TOGGLE, IDM_TAPE_FAST_TOGGLE,
		IDM_JOY_KEMPSTON, IDM_JOY_CURSOR, IDM_JOY_QAOP, IDM_JOY_SINCLAIR2
	};
	struct eUpdateJoyMenu
	{
		eUpdateJoyMenu()
		{
			CheckMenuItem(joy_menu, IDM_JOY_KEMPSTON, MF_BYCOMMAND|(OpJoystick() == J_KEMPSTON ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(joy_menu, IDM_JOY_CURSOR, MF_BYCOMMAND|(OpJoystick() == J_CURSOR ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(joy_menu, IDM_JOY_QAOP, MF_BYCOMMAND|(OpJoystick() == J_QAOP ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem(joy_menu, IDM_JOY_SINCLAIR2, MF_BYCOMMAND|(OpJoystick() == J_SINCLAIR2 ? MF_CHECKED : MF_UNCHECKED));
		}
	};

	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDM_OPEN_FILE:
			{
				OPENFILENAME ofn;
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize	= sizeof(ofn);
				ofn.hwndOwner = hWnd;
				wchar_t file[1024];
				file[0] = '\0';
				ofn.lpstrFile = file;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = resource_path;
				ofn.lpstrFilter = L"All supported formats\0*.sna;*.z80;*.szx;*.rzx;*.tap;*.csw;*.tzx;*.trd;*.scl;*.fdi;*.zip\0\0";
				ofn.Flags = OFN_PATHMUSTEXIST;
				if(GetOpenFileName(&ofn))
				{
					char buf[1024];
					WideCharToMultiByte(CP_ACP, 0, file, -1, buf, 1023, NULL, NULL);
					Handler()->OnOpenFile(buf);
				}
			}
			break;
		case IDM_TAPE_TOGGLE:
			Handler()->OnAction(A_TAPE_TOGGLE);
			break;
		case IDM_TAPE_FAST_TOGGLE:
			{
				using namespace xOptions;
				eOption<bool>* op_tape_fast = eOption<bool>::Find("fast tape");
				SAFE_CALL(op_tape_fast)->Change();
				bool tape_fast = op_tape_fast && *op_tape_fast;
				CheckMenuItem(main_menu, IDM_TAPE_FAST_TOGGLE, MF_BYCOMMAND|(tape_fast ? MF_CHECKED : MF_UNCHECKED));
			}
			break;
		case IDM_RESET:
			Handler()->OnAction(A_RESET);
			break;
		case IDM_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case IDM_JOY_KEMPSTON:
			OpJoystick(J_KEMPSTON);
			eUpdateJoyMenu();
			break;
		case IDM_JOY_CURSOR:
			OpJoystick(J_CURSOR);
			eUpdateJoyMenu();
			break;
		case IDM_JOY_QAOP:
			OpJoystick(J_QAOP);
			eUpdateJoyMenu();
			break;
		case IDM_JOY_SINCLAIR2:
			OpJoystick(J_SINCLAIR2);
			eUpdateJoyMenu();
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_CREATE:
		{
			HMENU menu = CreateMenu();
			main_menu = CreatePopupMenu();
			AppendMenu(menu, MF_STRING|MF_POPUP, (UINT)main_menu, TEXT("File"));
			AppendMenu(main_menu, MF_STRING, IDM_OPEN_FILE, TEXT("Open"));
			AppendMenu(main_menu, MF_STRING, IDM_TAPE_TOGGLE, TEXT("Start/stop tape"));
			xOptions::eOption<bool>* op_tape_fast = xOptions::eOption<bool>::Find("fast tape");
			AppendMenu(main_menu, MF_STRING|(op_tape_fast && *op_tape_fast ? MF_CHECKED : 0), IDM_TAPE_FAST_TOGGLE, TEXT("Fast tape"));

			joy_menu = CreatePopupMenu();
			AppendMenu(joy_menu, MF_STRING, IDM_JOY_KEMPSTON, TEXT("Kempston"));
			AppendMenu(joy_menu, MF_STRING, IDM_JOY_CURSOR, TEXT("Cursor"));
			AppendMenu(joy_menu, MF_STRING, IDM_JOY_QAOP, TEXT("QAOP"));
			AppendMenu(joy_menu, MF_STRING, IDM_JOY_SINCLAIR2, TEXT("Sinclair2"));
			eUpdateJoyMenu();

			AppendMenu(main_menu, MF_STRING|MF_POPUP, (UINT)joy_menu, TEXT("Joystick"));
			AppendMenu(main_menu, MF_STRING, IDM_RESET, TEXT("Reset"));
			AppendMenu(main_menu, MF_STRING, IDM_EXIT, TEXT("Exit"));
			SHMENUBARINFO mbi;
			memset(&mbi, 0, sizeof(SHMENUBARINFO));
			mbi.cbSize     = sizeof(SHMENUBARINFO);
			mbi.hwndParent = hWnd;
			mbi.nToolBarId = (UINT)menu;
			mbi.hInstRes   = g_hInst;
			mbi.dwFlags    = SHCMBF_HMENU;
			if(SHCreateMenuBar(&mbi)) 
				g_hWndMenuBar = mbi.hwndMB;
			else
			{
				g_hWndMenuBar = NULL;
				DestroyMenu(menu);
			}
		}
		// Initialize the shell activate info structure
		memset(&s_sai, 0, sizeof (s_sai));
		s_sai.cbSize = sizeof (s_sai);
		SetTimer(hWnd, TM_UPDATE, 20, NULL);
		break;
	case WM_TIMER:
		switch(wParam)
		{
		case TM_UPDATE:
			InvalidateRect(hWnd, NULL, false);
			return TRUE;
		}
		break;
	case WM_ERASEBKGND:
		return FALSE;
	case WM_PAINT:
		if(handler_inited)
		{
			Handler()->OnLoop();
			OnLoopSound();
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			if(!hdc_mem)
			{
				hdc_mem = CreateCompatibleDC(hdc);
				BITMAPINFO* bi = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFO) + 2*sizeof(RGBQUAD));
				bi->bmiHeader.biSize = sizeof(bi->bmiHeader);
				bi->bmiHeader.biWidth = 320;
				bi->bmiHeader.biHeight = 240;
				bi->bmiHeader.biPlanes = 1;
				bi->bmiHeader.biBitCount = 16;
				bi->bmiHeader.biCompression = BI_BITFIELDS;
				((PDWORD)bi->bmiColors)[0] = 0xf800;
				((PDWORD)bi->bmiColors)[1] = 0x07e0;
				((PDWORD)bi->bmiColors)[2] = 0x001f;
				bmp_mem = (HBITMAP)CreateDIBSection(hdc, bi, DIB_RGB_COLORS, (VOID**)&tex, 0, 0);
				SelectObject(hdc_mem, bmp_mem);
			}
			byte* data = (byte*)Handler()->VideoData();
			for(int y = 0; y < 240; ++y)
			{
				for(int x = 0; x < 320; ++x)
				{
					byte r, g, b;
					byte c = data[y*320+x];
					const byte brightness = 200;
					const byte bright_intensity = 55;
					byte i = c&8 ? brightness + bright_intensity : brightness;
					b = c&1 ? i : 0;
					r = c&2 ? i : 0;
					g = c&4 ? i : 0;
					word* p = &tex[(239 - y)*320 + x];
					*p = RGBX(r, g ,b);
				}
			}
			BitBlt(hdc, 0, 0, 320, 240, hdc_mem, 0, 0, SRCCOPY);
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			int vk_key = (int)wParam;
			char key = 0;
			dword flags = KF_DOWN|OpJoyKeyFlags();
			if(GetKeyState(VK_MENU))	flags |= KF_ALT;
			if(GetKeyState(VK_SHIFT))	flags |= KF_SHIFT;
			TranslateKey(vk_key, key, flags);
			Handler()->OnKey(key, flags);
			Handler()->OnLoop();
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			int vk_key = (int)wParam;
			char key = 0;
			dword flags = 0;
			if(GetKeyState(VK_MENU))	flags |= KF_ALT;
			if(GetKeyState(VK_SHIFT))	flags |= KF_SHIFT;
			TranslateKey(vk_key, key, flags);
			Handler()->OnKey(key, OpJoyKeyFlags());
			Handler()->OnLoop();
		}
		break;

	case WM_DESTROY:
		if(hdc_mem)
		{
			DeleteDC(hdc_mem);
			DeleteObject(bmp_mem);
		}
		CommandBar_Destroy(g_hWndMenuBar);
		PostQuitMessage(0);
		break;

	case WM_ACTIVATE:
		// Notify shell of our activate message
		SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
		break;
	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

}
//namespace xPlatform

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	if(xPlatform::Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
	{
		xPlatform::Loop();
	}
	xPlatform::Done();
}

#endif//_WIN_MOBILE
