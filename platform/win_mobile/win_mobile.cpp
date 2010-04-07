#include "../../std.h"

#ifdef _WIN_MOBILE

#include <windows.h>
#include <aygshell.h>
#pragma comment(lib, "aygshell.lib") 

#include "../platform.h"
#include "../io.h"

namespace xPlatform
{

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
static const byte brightness = 200;
static const byte bright_intensity = 55;

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
	int wmId, wmEvent;
	static SHACTIVATEINFO s_sai;
	static HDC hdc_mem = NULL;
	static HBITMAP bmp_mem = NULL;
	static word* tex = NULL;

	enum eTimerId { TM_UPDATE = 1 };
	enum eCommandId { IDM_EXIT = 40000, IDM_OPEN_FILE, IDM_RESET, IDM_TAPE_TOGGLE };

	switch(message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch(wmId)
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
				ofn.lpstrFilter = L"All supported formats\0*.sna;*.tap;*.tzx;*.trd;*.scl\0\0";
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
		case IDM_RESET:
			Handler()->OnAction(A_RESET);
			break;
		case IDM_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_CREATE:
		{
			HMENU menu = CreateMenu();
			HMENU p = CreatePopupMenu();
			AppendMenu(menu, MF_STRING|MF_POPUP, (UINT)p, TEXT ("File"));
			AppendMenu(p, MF_STRING, IDM_OPEN_FILE, TEXT ("Open"));
			AppendMenu(p, MF_STRING, IDM_TAPE_TOGGLE, TEXT ("Start/stop tape"));
			AppendMenu(p, MF_STRING, IDM_RESET, TEXT ("Reset"));
			AppendMenu(p, MF_STRING, IDM_EXIT, TEXT ("Exit"));
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
			dword flags = KF_DOWN|KF_CURSOR|KF_KEMPSTON;
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
			Handler()->OnKey(key, KF_CURSOR|KF_KEMPSTON);
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
