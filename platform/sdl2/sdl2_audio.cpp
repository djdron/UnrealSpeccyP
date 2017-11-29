/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
#include "../../tools/options.h"
#include "../../tools/sound_mixer.h"

namespace xPlatform
{

static SDL_AudioDeviceID device = 0;

static eSoundMixer sound_mixer;

static void AudioCallback(void* userdata, Uint8* stream, int len)
{
	if((dword)len <= sound_mixer.Ready())
	{
		memcpy(stream, sound_mixer.Ptr(), len);
		sound_mixer.Use(len);
	}
	else
	{
		memcpy(stream, sound_mixer.Ptr(), sound_mixer.Ready());
		memset(stream + sound_mixer.Ready(), 0, len - sound_mixer.Ready());
		sound_mixer.Use(sound_mixer.Ready());
	}
}

bool InitAudio()
{
	using namespace xOptions;
	struct eOptionBX : public eOptionB
	{
		void Unuse() { customizable = false; storeable = false; }
	};
	eOptionBX* o = (eOptionBX*)eOptionB::Find("sound");
	SAFE_CALL(o)->Unuse();
	o = (eOptionBX*)eOptionB::Find("volume");
	SAFE_CALL(o)->Unuse();

	SDL_AudioSpec audio;
	memset(&audio, 0, sizeof(audio));
	audio.freq = 44100;
	audio.channels = 2;
	audio.format = AUDIO_S16;
#ifndef SDL_AUDIO_SAMPLES
#define SDL_AUDIO_SAMPLES 1024
#endif//SDL_AUDIO_SAMPLES
	audio.samples = SDL_AUDIO_SAMPLES;
	audio.callback = AudioCallback;
	SDL_AudioSpec obtained;
	device = SDL_OpenAudioDevice(NULL, 0, &audio, &obtained, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if(!device)
		return false;
	Handler()->AudioSetSampleRate(obtained.freq);
	SDL_PauseAudioDevice(device, 0);
	return true;
}
void DoneAudio()
{
	if(device)
	{
		SDL_PauseAudioDevice(device, 1);
		SDL_CloseAudioDevice(device);
		device = 0;
	}
}

void UpdateAudio()
{
	if(!device)
		return;

	SDL_LockAudioDevice(device);
	sound_mixer.Update();
	static bool audio_filled = false;
	bool audio_filled_new = sound_mixer.Ready() > 44100*2*2/50*7; // 7-frame data
	if(audio_filled != audio_filled_new)
	{
		audio_filled = audio_filled_new;
		Handler()->VideoPaused(audio_filled);
	}
	SDL_UnlockAudioDevice(device);
}

}
//namespace xPlatform

#endif//USE_SDL2
