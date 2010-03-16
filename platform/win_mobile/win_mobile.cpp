#include "../../std.h"
#include "../platform.h"
#include "../io.h"

#ifdef _WIN_MOBILE

#include <windows.h>

namespace xPlatform
{
void Init()
{
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
}
void Done()
{
	Handler()->OnDone();
}
void Loop()
{
	for(;;)
	{
		Handler()->OnLoop();
	}
}

}
//namespace xPlatform

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	xPlatform::Init();
	xPlatform::Loop();
	xPlatform::Done();
}

#endif//_WIN_MOBILE
