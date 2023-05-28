/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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
#include "../../options_common.h"
#include "../../tools/tick.h"
#include "../io.h"

#include <AppKit.h>
#include <StorageKit.h>
#include <FindDirectory.h>

#include "haiku_app.h"

namespace xPlatform
{

void InitAudio();
void DoneAudio();
void UpdateAudio();

bool InitVideo();
void DoneVideo();
void UpdateScreen();

bool InitJoystick();
void DoneJoystick();
void ProcessJoystick();

static int32 appThreadFunc(void *data)
{
	USPApplication *application = new USPApplication(APP_SIGNATURE);

	app_info theInfo;
	application->GetAppInfo(&theInfo);
	BPath appPath(&theInfo.ref);
	BPath dirPath;
	appPath.GetParent(&dirPath);
	chdir(dirPath.Path());

	application->Run();

	return 0;
}

static bool Init()
{
	thread_id appThreadId = spawn_thread(appThreadFunc, "USPApplication", B_NORMAL_PRIORITY, NULL);
	resume_thread(appThreadId);
	
	while(true) {
		if (be_app) {
			if (!be_app->IsLaunching())
				break;
		}
		snooze(1000);
	}

    BPath settings_path;
    if (find_directory(B_USER_SETTINGS_DIRECTORY, &settings_path) == B_OK) {
    	settings_path.Append("USP");
    	BString usp_home_path(settings_path.Path());
    	usp_home_path += "/";
		xIo::PathCreate(usp_home_path.String());
		xIo::SetProfilePath(usp_home_path.String());
		OpLastFile(usp_home_path.String());
 	}

	Handler()->OnInit();

	InitJoystick();
	InitAudio();
	InitVideo();

	return true;
}

static void Done()
{
	DoneJoystick();
	DoneAudio();
	DoneVideo();

	Handler()->OnDone();
}

static bool quit = false;

void Loop()
{
	eTick last_tick;
	last_tick.SetCurrent();
	while(!quit) {
		Handler()->OnLoop();
		UpdateScreen();
		ProcessJoystick();
		UpdateAudio();

		while(last_tick.Passed().Ms() < 15)
			snooze(3000);

		last_tick.SetCurrent();
		if(OpQuit())
			quit = true;
	}
}

}

int main(int argc, char* argv[])
{
	if(!xPlatform::Init()) {
		xPlatform::Done();
		return -1;
	}

	if(argc > 1)
		xPlatform::Handler()->OnOpenFile(argv[1]);

	xPlatform::Loop();	
	xPlatform::Done();

	return 0;
}
