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

#ifdef USE_QT

#include "qt_sound.h"
#include "../platform.h"

//=============================================================================
//	eAudioBuffer::Fill
//-----------------------------------------------------------------------------
void eAudioBuffer::Fill(const void* data, dword size)
{
	if(ready + size < BUF_SIZE)
	{
		memcpy(buffer + ready, data, size);
		ready += size;
	}
	else
		ready = 0;
}
//=============================================================================
//	eAudioBuffer::Use
//-----------------------------------------------------------------------------
void eAudioBuffer::Use(dword size)
{
	if(size)
	{
		if(ready > size)
			memmove(buffer, buffer + size, ready - size);
		ready -= size;
	}
}
//=============================================================================
//	eAudioBuffer::Update
//-----------------------------------------------------------------------------
void eAudioBuffer::Update(int active_sound_src)
{
	using namespace xPlatform;
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		if(i == active_sound_src && !Handler()->FullSpeed())
		{
			Fill(Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
	static bool video_paused = false;
	bool video_paused_new = Ready() > (44100*2*2/50)*3; // 3-frame audio data
	if(video_paused_new != video_paused)
	{
		video_paused = video_paused_new;
		Handler()->VideoPaused(video_paused);
	}
}

#endif//USE_QT
