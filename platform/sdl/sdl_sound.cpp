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

class eAudioBuffer
{
public:
	enum { SIZE = 65536 };
	eAudioBuffer()
	{
		begin = data;
		end = begin + SIZE;
		current = ready = begin;
	}
	void Add(byte* data, size_t size) { memcpy(ready, data, size); ready += size; }
	byte* Current() const { return current; }
	size_t Ready() const { return ready - current; }
	void Use(size_t size) { current += size; Purge(); }

protected:
	void Purge()
	{
		if(current - begin > SIZE/2)
		{
			size_t size = Ready();
			memcpy(begin, current, size);
			current = begin;
			ready = begin + size;
		}
	}

protected:
	byte* begin;
	byte* end;
	byte* current;
	byte* ready;
	byte data[SIZE];
};

static eAudioBuffer* audio_buffer = NULL;

static void AudioCallback(void* userdata, Uint8* stream, int len)
{
	if(audio_buffer->Ready() >= (size_t)len)
	{
		memcpy(stream, audio_buffer->Current(), len);
		audio_buffer->Use(len);
	}
}

bool InitAudio()
{
	SDL_AudioSpec audio;
	memset(&audio, 0, sizeof(audio));
	audio.freq = 44100;
	audio.channels = 2;
	audio.format = AUDIO_S16SYS;
	audio.samples = 4096;
	audio.callback = AudioCallback;
	if(SDL_OpenAudio(&audio, NULL) < 0)
		return false;
	audio_buffer = new eAudioBuffer;
	SDL_PauseAudio(0);
	return true;
}
void DoneAudio()
{
	SDL_PauseAudio(1);
	SAFE_DELETE(audio_buffer);
}

void UpdateAudio()
{
	SDL_LockAudio();
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		bool ui_enabled = Handler()->VideoDataUI();
		if(size && i == OpSound() && !ui_enabled && !Handler()->FullSpeed())
		{
			audio_buffer->Add((byte*)Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
	static bool audio_filled = false;
	bool audio_filled_new = audio_buffer->Ready() > audio_buffer->SIZE/4;
	if(audio_filled != audio_filled_new)
	{
		audio_filled = audio_filled_new;
		Handler()->VideoPaused(audio_filled);
	}
	SDL_UnlockAudio();
}

}
//namespace xPlatform

#endif//USE_SDL
