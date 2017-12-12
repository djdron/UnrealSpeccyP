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
#include "../../tools/tick.h"

#ifdef USE_LIBRARY

#ifdef _WINDOWS
#define USP_API __declspec(dllexport)
#else
#define USP_API
#endif//

extern "C"
{

using namespace xPlatform;
USP_API void Init()
{
	Handler()->OnInit();
}

USP_API void Loop()
{
	Handler()->OnLoop();
}

USP_API void Done()
{
	Handler()->OnDone();
}

USP_API void Open(const char* name)
{
	Handler()->OnOpenFile(name);
}

USP_API void OnKey(char key, dword flags)
{
	Handler()->OnKey(key, flags);
}

}
//extern "C"

#endif//USE_LIBRARY
