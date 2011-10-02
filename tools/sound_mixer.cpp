/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "sound_mixer.h"
#include "../std.h"
#include "../platform/platform.h"

//=============================================================================
//	eSoundMixer::Update
//-----------------------------------------------------------------------------
void eSoundMixer::Update(byte* ext_buf)
{
	using namespace xPlatform;
	int x = Handler()->AudioSources();
	if(!x)
		return;
	assert(x == 3); // for some optimizations
	dword ready_min = Handler()->AudioDataReady(0);
	for(int s = 1; s < x; ++s)
	{
		dword ready_s = Handler()->AudioDataReady(s);
		if(ready_min > ready_s)
			ready_min = ready_s;
	}
	if(ready + ready_min > BUF_SIZE)
	{
		ready = 0;
		ready_min = 0;
	}
	if(ready_min)
	{
		byte* buf = ext_buf ? ext_buf : buffer;
		int* p = (int*)(buf + ready);
		int* s0 = (int*)Handler()->AudioData(0);
		int* s1 = (int*)Handler()->AudioData(1);
		int* s2 = (int*)Handler()->AudioData(2);
		for(int i = ready_min/4; --i >= 0;)
		{
			if(i > 8)
			{
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				*p++ = (*s0++) + (*s1++) + (*s2++);
				i -= 8;
			}
			*p++ = (*s0++) + (*s1++) + (*s2++);
		}
		ready += ready_min;
	}
	for(int s = 0; s < x; ++s)
	{
		Handler()->AudioDataUse(s, Handler()->AudioDataReady(s));
	}
}
//=============================================================================
//	eSoundMixer::Use
//-----------------------------------------------------------------------------
void eSoundMixer::Use(dword size, byte* ext_buf)
{
	if(size)
	{
		if(ready > size)
		{
			byte* buf = ext_buf ? ext_buf : buffer;
			memmove(buf, buf + size, ready - size);
		}
		ready -= size;
	}
}
