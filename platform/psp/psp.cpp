/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _PSP

#include <pspkernel.h>
#include <pspctrl.h>
#include "../io.h"
#include "../platform.h"

PSP_MODULE_INFO("Unreal Speccy Portable", 0, 1, 1);
PSP_HEAP_SIZE_KB(-1024);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

static bool running = true;
static int exitCallback(int arg1, int arg2, void* common)
{
	running = false;
	return 0;
}
static int callbackThread(SceSize args, void* argp)
{
	int cbid = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}
static int setupCallbacks()
{
	int thid = sceKernelCreateThread("update_thread", callbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return thid;
}

namespace xPlatform
{

void InitVideo();
void DoneVideo();
void UpdateScreen();
void InitAudio();
void DoneAudio();
void UpdateAudio();
void UpdateKeys();

void Init(const char* app_path)
{
	setupCallbacks();
	char app_dir[xIo::MAX_PATH_LEN];
	char* pos = strrchr(app_path, '/');
	if(pos)
	{
		strncpy(app_dir, app_path, pos - app_path + 1);
		app_dir[pos - app_path + 1] = '\0';
		xIo::SetResourcePath(app_dir);
		xIo::SetProfilePath(app_dir);
	}
	Handler()->OnInit();
	InitVideo();
	InitAudio();
}

void Done()
{
	DoneAudio();
	DoneVideo();
	Handler()->OnDone();
	sceKernelExitGame();
}

void Loop()
{
	while(running)
	{
		UpdateKeys();
		Handler()->OnLoop();
		UpdateAudio();
		UpdateScreen();
	}
}

}
//namespace xPlatform

int main(int argc, char* argv[])
{
	xPlatform::Init(argv[0]);
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

#endif//_PSP
