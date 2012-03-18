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

#ifdef _CHROME_NACL

#include "ppapi/cpp/audio_config.h"
#include "nacl_sound.h"
#include "../platform.h"

namespace xPlatform
{

void eAudio::Init(pp::Instance* instance)
{
	samples_count = pp::AudioConfig::RecommendSampleFrameCount(PP_AUDIOSAMPLERATE_44100, 2048);
	pp::AudioConfig audio_config = pp::AudioConfig(instance, PP_AUDIOSAMPLERATE_44100, samples_count);
	impl = pp::Audio(instance, audio_config, _GetData, this);
}

void eAudio::Update()
{
	pthread_mutex_lock(&mutex);
	mixer.Update();
	static bool video_paused = false;
	bool video_paused_new = mixer.Ready() > (samples_count*2*sizeof(int16_t))*2;
	if(video_paused_new != video_paused)
	{
		video_paused = video_paused_new;
		Handler()->VideoPaused(video_paused);
	}
	pthread_mutex_unlock(&mutex);
}

void eAudio::GetData(void* _samples, uint32_t buffer_size)
{
	pthread_mutex_lock(&mutex);
	dword size = samples_count*2*sizeof(int16_t);
	assert(size <= buffer_size);
	byte* buf = (byte*)_samples;
	if(size <= mixer.Ready())
	{
		memcpy(buf, mixer.Ptr(), size);
		mixer.Use(size);
	}
	else
	{
		memcpy(buf, mixer.Ptr(), mixer.Ready());
		memset(buf + mixer.Ready(), 0, size - mixer.Ready());
		mixer.Use(mixer.Ready());
	}
	pthread_mutex_unlock(&mutex);
}

}
//namespace xPlatform

#endif//_CHROME_NACL
