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

#ifdef USE_OAL

#ifdef _WINDOWS
#include <al.h>
#include <alc.h>
#elif defined(_MAC) || defined(_IOS)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else//_MAC
#include <AL/al.h>
#include <AL/alc.h>
#endif//_WINDOWS

#include "../../tools/options.h"
#include "../../tools/sound_mixer.h"

namespace xPlatform
{

void InitSound();
void DoneSound();

static struct eOptionTrueSpeed : public xOptions::eOptionBool
{
	eOptionTrueSpeed() { Set(true); }
	virtual const char* Name() const { return "true speed"; }
	virtual void Change(bool next = true)
	{
		eOptionBool::Change();
		DoneSound();
		InitSound();
	}
	virtual int Order() const { return 75; }
} op_true_speed;

struct eSource
{
	void Clear()
	{
		source = 0;
		free_buf = 0;
		first_fill = true;
		memset(buffers, 0, sizeof(buffers));
	}
	void Init()
	{
		Clear();
		alGetError();
		alGenBuffers(BUFFER_COUNT, buffers);
		alGetError();
		alGenSources(1, &source);
	}
	void Done()
	{
		alSourceStop(source);
		alDeleteSources(1, &source);
		alDeleteBuffers(BUFFER_COUNT, buffers);
		Clear();
	}
	enum eUpdateResult { U_OK, U_SKIP, U_MUCH, U_LESS  };
	eUpdateResult Update();
	enum { BUFFER_COUNT = 4 };
	ALuint buffers[BUFFER_COUNT];
	ALuint source;
	ALuint free_buf;
	bool first_fill;
	eSoundMixer mixer;
};
eSource::eUpdateResult eSource::Update()
{
	mixer.Update();
	const float fps = op_true_speed ? 50.0f : 60.0f;
	const float fps_org = 50.0f;
	dword frame_data = 44100*2*2/fps_org;
	dword data_ready = mixer.Ready();
	if(data_ready < frame_data*2)
		return U_LESS;

	bool next_buf = first_fill;
	if(!next_buf)
	{
		ALint buffers_processed = 0;
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &buffers_processed);
		if(buffers_processed)
		{
			ALuint buf = 0;
			alSourceUnqueueBuffers(source, 1, &buf);
			assert(buf == buffers[free_buf]);
			next_buf = true;
		}
	}
	if(next_buf)
	{
		alBufferData(buffers[free_buf], AL_FORMAT_STEREO16, mixer.Ptr(), data_ready, 44100*fps/fps_org);
		mixer.Use(data_ready);
		alSourceQueueBuffers(source, 1, &buffers[free_buf]);
		if(++free_buf == BUFFER_COUNT)
		{
			free_buf = 0;
			first_fill = false;
		}
	}
	ALint state = 0;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	if(state != AL_PLAYING)
	{
		ALint queued_buffers = 0;
		alGetSourcei(source, AL_BUFFERS_QUEUED, &queued_buffers);
		if(queued_buffers)
		{
			alSourcePlay(source);
		}
	}
	return next_buf ? U_OK : (data_ready > frame_data*3 ? U_MUCH : U_SKIP);
}

static eSource source_mixed;
static ALCdevice* device = NULL;
static ALCcontext* context = NULL;

void InitSound()
{
	device = alcOpenDevice(NULL);
	if(!device)
		return;
	context = alcCreateContext(device, NULL);
	if(!context)
		return;
	alcMakeContextCurrent(context);
	alcProcessContext(context);
	source_mixed.Init();
}

void DoneSound()
{
	if(device && context)
	{
		source_mixed.Done();
	}
	if(context)
	{
		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		context = NULL;
	}
	if(device)
	{
		alcCloseDevice(device);
		device = NULL;
	}
}

void OnLoopSound()
{
	if(!device || !context)
		return;
	static bool video_paused = false;
	bool video_paused_new = source_mixed.Update() == eSource::U_MUCH;
	if(video_paused_new != video_paused)
	{
		video_paused = video_paused_new;
		Handler()->VideoPaused(video_paused);
	}
}

}
//namespace xPlatform

#endif//USE_OAL
