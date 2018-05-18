/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2018 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_SDL2

#include <SDL.h>
#include "../../options_common.h"
#include "../../tools/tick.h"
#include "../io.h"

namespace xPlatform
{

static bool sdl_inited = false;

bool InitVideo();
bool InitAudio();
void DoneVideo();
void DoneAudio();
void UpdateAudio();
void UpdateScreen();
void ProcessKey(SDL_Event& e);

#ifdef SDL_USE_MOUSE
void ProcessMouse(SDL_Event& e);
#endif//SDL_USE_MOUSE

#ifdef SDL_USE_JOYSTICK
void ProcessJoy(SDL_Event& e);
static SDL_Joystick* joystick = NULL;
#endif//SDL_USE_JOYSTICK

#ifndef SDL_DEFAULT_FOLDER
static const char* GetHomePath()
{
	static char home_path[xIo::MAX_PATH_LEN];
	const char* h = getenv("HOME");
	if(h)
	{
		strcpy(home_path, h);
		return home_path;
	}
	const char* hd = getenv("HOMEDRIVE"), *hp = getenv("HOMEPATH");
	if(hd && hp)
	{
		strcpy(home_path, hd);
		strcat(home_path, hp);
		return home_path;
	}
	return NULL;
}

static const char* USP_HomePath()
{
	static char usp_home_path[xIo::MAX_PATH_LEN];
	const char* h = GetHomePath();
	if(h)
	{
		strcpy(usp_home_path, h);
		strcat(usp_home_path, "/.usp/");
		return usp_home_path;
	}
	return NULL;
}
#endif//SDL_DEFAULT_FOLDER

static bool Init()
{
#ifndef SDL_DEFAULT_FOLDER
	const char* usp_home_path = USP_HomePath();
	if(usp_home_path)
	{
		xIo::PathCreate(usp_home_path);
		xIo::SetProfilePath(usp_home_path);
//		xIo::SetRootPath(usp_home_path);
		OpLastFile(usp_home_path);
	}
#else//SDL_DEFAULT_FOLDER
	xIo::SetProfilePath(SDL_DEFAULT_FOLDER);
	OpLastFile(SDL_DEFAULT_FOLDER);
#endif//SDL_DEFAULT_FOLDER
	Handler()->OnInit();

	Uint32 init_flags = SDL_INIT_VIDEO|SDL_INIT_AUDIO;
#ifdef SDL_USE_JOYSTICK
	init_flags |= SDL_INIT_JOYSTICK;
#endif//SDL_USE_JOYSTICK
	if(SDL_Init(init_flags) < 0)
        return false;

#ifdef SDL_USE_JOYSTICK
	SDL_JoystickEventState(SDL_ENABLE);
	joystick = SDL_JoystickOpen(0);
#endif//SDL_USE_JOYSTICK

    sdl_inited = true;

	if(!InitVideo())
		return false;
	if(!InitAudio())
		return false;
	return true;
}

void Done()
{
#ifdef SDL_USE_JOYSTICK
	if(joystick)
	{
		SDL_JoystickClose(joystick);
		joystick = NULL;
	}
#endif//SDL_USE_JOYSTICK
	DoneAudio();
	DoneVideo();
	if(sdl_inited)
		SDL_Quit();
	Handler()->OnDone();
}

static bool quit = false;

void Loop1()
{
	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		switch(e.type)
		{
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			ProcessKey(e);
			break;
#ifdef SDL_USE_MOUSE
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEMOTION:
			ProcessMouse(e);
			break;
#endif//SDL_USE_MOUSE
#ifdef SDL_USE_JOYSTICK
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
			ProcessJoy(e);
			break;
#endif//SDL_USE_JOYSTICK
		case SDL_DROPFILE:
			Handler()->OnOpenFile(e.drop.file);
			break;
		default:
			break;
		}
	}
	Handler()->OnLoop();
	UpdateScreen();
	UpdateAudio();
}

void Loop();

#ifndef SDL_NO_MAINLOOP
void Loop()
{
	eTick last_tick;
	last_tick.SetCurrent();
	while(!quit)
	{
		Loop1();
		while(last_tick.Passed().Ms() < 15)
		{
			SDL_Delay(3);
		}
		last_tick.SetCurrent();
		if(OpQuit())
			quit = true;
	}
}
#endif//SDL_NO_MAINLOOP

}
//namespace xPlatform

int main(int argc, char* argv[])
{
	if(!xPlatform::Init())
	{
		xPlatform::Done();
		return -1;
	}
	if(argc > 1)
		xPlatform::Handler()->OnOpenFile(argv[1]);
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

#endif//USE_SDL2
