#include "../../std.h"
#include "../platform.h"

#ifdef _WIN_MOBILE

#include <windows.h>

namespace xPlatform
{
void Init()
{
}
void Done()
{
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
