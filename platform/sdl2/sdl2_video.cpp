/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include <SDL2/SDL.h>
#include "../gles2/gles2.h"

namespace xPlatform
{

static SDL_Window* window = NULL;
static SDL_GLContext context = NULL;
static eGLES2* gles2 = NULL;

bool InitVideo()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0); 

	window = SDL_CreateWindow(Handler()->WindowCaption(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				852, 480, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if(!window)
		return false;
	context = SDL_GL_CreateContext(window);
	if(!context)
		return false;
	gles2 = eGLES2::Create();
	return true;
}

void DoneVideo()
{
	SAFE_DELETE(gles2);
	if(context)
	{
		SDL_GL_DeleteContext(context);
		context = NULL;
	}
	if(window)
	{
		SDL_DestroyWindow(window);
		window = NULL;
	}
}

void UpdateScreen()
{
	ePoint s;
	SDL_GetWindowSize(window, &s.x, &s.y);
	gles2->Draw(ZERO, s);
	SDL_GL_SwapWindow(window);
}

}
//namespace xPlatform

#endif//USE_SDL2
