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

#include <pspaudio.h>
#include <pspaudiolib.h>
#include <pspkernel.h>
#include "../platform.h"
#include "../../tools/sound_mixer.h"

namespace xPlatform
{

class eSimpleMutex
{
public:
	eSimpleMutex(const char* name)
	{
		handle = sceKernelCreateSema(name, 0, 1, 1, 0);
	}
	~eSimpleMutex()
	{
		sceKernelDeleteSema(handle);
	}
	void Lock()
	{
		sceKernelWaitSema(handle, 1, 0);
	}
	void Unlock()
	{
		sceKernelSignalSema(handle, 1);
	}
protected:
	SceUID handle;
};

class eAutoMutex
{
public:
	eAutoMutex(eSimpleMutex& _m) : m(_m) { m.Lock(); }
	~eAutoMutex() { m.Unlock(); }
protected:
	eSimpleMutex& m;
};

static eSimpleMutex sound_mutex("sound_mutex");

static eSoundMixer sound_mixer;

static void AudioCallback(void* buf, unsigned int length, void* userdata)
{
	eAutoMutex lock(sound_mutex);
	length *= 4; // translate samples to bytes
	if(length <= sound_mixer.Ready())
	{
		memcpy(buf, sound_mixer.Ptr(), length);
		sound_mixer.Use(length);
	}
	else
	{
		memcpy(buf, sound_mixer.Ptr(), sound_mixer.Ready());
		memset((byte*)buf + sound_mixer.Ready(), 0, length - sound_mixer.Ready());
		sound_mixer.Use(sound_mixer.Ready());
	}
}

void InitAudio()
{
	pspAudioInit();
	pspAudioSetChannelCallback(0, AudioCallback, NULL);
}

void UpdateAudio()
{
	eAutoMutex lock(sound_mutex);
	sound_mixer.Update();
	static bool audio_filled = false;
	bool audio_filled_new = sound_mixer.Ready() > 44100*2*2/50*7; // 7-frame data
	if(audio_filled != audio_filled_new)
	{
		audio_filled = audio_filled_new;
		Handler()->VideoPaused(audio_filled);
	}
}

void DoneAudio()
{
	pspAudioEnd();
}

}
//namespace xPlatform

#endif//_PSP
