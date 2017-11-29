/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2017 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
#ifdef SDL_USE_MOUSE

#include <SDL.h>
#include "../../tools/point.h"

namespace xPlatform
{

extern SDL_Window* window;
void OpZoomGet(float* sx, float* sy, const ePoint& org_size, const ePoint& size);

struct eMouseDelta
{
	eMouseDelta() : x(0.0f), y(0.0f) {}
	eMouseDelta(const ePoint& d, float sx, float sy)
	{
		x = sx*d.x;
		y = sy*d.y;
	}
	eMouseDelta& operator+=(const eMouseDelta& d) { x += d.x; y += d.y; return *this; }
	float x;
	float y;
};
static eMouseDelta mouse_delta;

bool ProcessMouseGrab(SDL_Event& e)
{
	if(e.type != SDL_KEYDOWN)
		return false;
	switch(e.key.keysym.sym)
	{
	case SDLK_ESCAPE:
		if(SDL_GetWindowGrab(window))
		{
			SDL_SetWindowGrab(window, SDL_FALSE);
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		}
		break;
	}
	return false;
}

void ProcessMouse(SDL_Event& e)
{
	switch(e.type)
	{
	case SDL_MOUSEBUTTONDOWN:
		if(SDL_GetWindowGrab(window))
			Handler()->OnMouse(MA_BUTTON, e.button.button == 1 ? 0 : 1, 1);
		else
		{
			SDL_SetWindowGrab(window, SDL_TRUE);
			SDL_SetRelativeMouseMode(SDL_TRUE);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if(SDL_GetWindowGrab(window))
			Handler()->OnMouse(MA_BUTTON, e.button.button == 1 ? 0 : 1, 0);
		break;
	case SDL_MOUSEMOTION:
		if(SDL_GetWindowGrab(window))
		{
			ePoint size;
			SDL_GetWindowSize(window, &size.x, &size.y);
			float sx, sy;
			OpZoomGet(&sx, &sy, ePoint(320, 240), size);
			float scale_x = 320.0f/size.x/sx;
			float scale_y = 240.0f/size.y/sy;
			mouse_delta += eMouseDelta(ePoint(e.motion.xrel, e.motion.yrel), scale_x, scale_y);
			int dx = mouse_delta.x;
			int dy = mouse_delta.y;
			if(dx || dy)
			{
				mouse_delta.x -= dx;
				mouse_delta.y -= dy;
				Handler()->OnMouse(MA_MOVE, dx, -dy);
			}
		}
		break;
	}
}

}
//namespace xPlatform

#endif//SDL_USE_MOUSE
#endif//USE_SDL2
