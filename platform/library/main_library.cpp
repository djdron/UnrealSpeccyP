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
#include "../../speccy.h"
#include "../../devices/memory.h"

#ifdef USE_LIBRARY

#ifdef _WINDOWS
#define USP_API __declspec(dllexport)
#else
#define USP_API
#endif//

extern "C"
{

using namespace xPlatform;
USP_API void USP_Init()
{
	Handler()->OnInit();
}

USP_API void USP_Loop()
{
	Handler()->OnLoop();
}

USP_API void USP_Done()
{
	Handler()->OnDone();
}

USP_API void USP_OpenFile(const char* name)
{
	Handler()->OnOpenFile(name);
}

USP_API void USP_OnKey(char key, dword flags)
{
	Handler()->OnKey(key, flags);
}

USP_API void USP_GetVideoData(byte buf[320*240])
{
	memcpy(buf, Handler()->VideoData(), 320*240);
}

USP_API void USP_MemoryRead(byte* buf, dword addr, dword size)
{
	eSpeccy* s = Handler()->Speccy();
	byte* src = s->Memory()->Get(0) + addr;
	memcpy(buf, src, size);
}

USP_API void USP_MemoryWrite(const byte* buf, dword addr, dword size)
{
	eSpeccy* s = Handler()->Speccy();
	byte* dst = s->Memory()->Get(0) + addr;
	memcpy(dst, buf, size);
}

}
//extern "C"

#endif//USE_LIBRARY
