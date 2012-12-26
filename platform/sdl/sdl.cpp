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

#ifdef USE_SDL

#include <SDL.h>
#include "../../options_common.h"

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

#ifdef SDL_USE_JOYSTICK
void ProcessJoy(SDL_Event& e);
static SDL_Joystick* joystick = NULL;
#endif//SDL_USE_JOYSTICK

static bool Init()
{
#ifndef SDL_DEFAULT_FOLDER
#define SDL_DEFAULT_FOLDER "/"
#endif//SDL_DEFAULT_FOLDER
	OPTION_GET(op_last_file)->Set((const char*&)SDL_DEFAULT_FOLDER);
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
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(Handler()->WindowCaption(), NULL);

	if(!InitVideo())
		return false;
	if(!InitAudio())
		return false;
	return true;
}

static void Done()
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

static void Loop()
{
	bool quit = false;
	while(!quit)
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
#ifdef SDL_USE_JOYSTICK
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				ProcessJoy(e);
				break;
#endif//SDL_USE_JOYSTICK
			default:
				break;
			}
		}
		Handler()->OnLoop();
		UpdateScreen();
		UpdateAudio();
//		SDL_Delay(5);
		if(*OPTION_GET(op_quit))
			quit = true;
	}
}

}
//namespace xPlatform

int main(int argc, char* argv[])
{
	if(!xPlatform::Init())
	{
		xPlatform::Done();
		return -1;
	}
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

#endif//USE_SDL
