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

bool Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	// Perform application initialization:
	if(!InitInstance(hInstance, nCmdShow)) 
	{
		return false;
	}
	wchar_t buf_w[1024];
	int l = GetModuleFileName(NULL, buf_w, 1024);
	for(; --l >= 0 && buf_w[l] != '\\'; )
	{
	}
	buf_w[++l] = '\0';
	char buf[1024];
	WideCharToMultiByte(CP_ACP, 0, buf_w, -1, buf, l, NULL, NULL);
	xIo::SetResourcePath(buf);
	Handler()->OnInit();
	return true;
}
void Done()
{
	Handler()->OnDone();
}

// Global Variables:
HINSTANCE			g_hInst;			// current instance
HWND				g_hWndMenuBar;		// menu bar handle

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
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

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

#define RGBX(r, g, b)	((b << 8)|(g << 3)|(r >> 3))
static const byte brightness = 200;
static const byte bright_intensity = 55;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	static SHACTIVATEINFO s_sai;
	static HDC hdc_mem = NULL;
	static HBITMAP bmp_mem = NULL;
	static word* tex = NULL;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
// 		switch (wmId)
// 		{
// 		case IDM_HELP_ABOUT:
// 			DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, About);
// 			break;
// 		case IDM_OK:
// 			SendMessage (hWnd, WM_CLOSE, 0, 0);				
// 			break;
// 		default:
// 			return DefWindowProc(hWnd, message, wParam, lParam);
//		}
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	case WM_CREATE:
		SHMENUBARINFO mbi;

		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize     = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hWnd;
		mbi.nToolBarId = 0;//IDR_MENU;
		mbi.hInstRes   = g_hInst;

		if (!SHCreateMenuBar(&mbi)) 
		{
			g_hWndMenuBar = NULL;
		}
		else
		{
			g_hWndMenuBar = mbi.hwndMB;
		}

		// Initialize the shell activate info structure
		memset(&s_sai, 0, sizeof (s_sai));
		s_sai.cbSize = sizeof (s_sai);
		SetTimer(hWnd, 1, 20, NULL);
		break;
	case WM_TIMER:
		InvalidateRect(hWnd, NULL, false);
		return TRUE;
	case WM_ERASEBKGND:
		return FALSE;
	case WM_PAINT:
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

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		{
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, message);
		return TRUE;

	}
	return (INT_PTR)FALSE;
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
