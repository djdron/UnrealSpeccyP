#include "../../std.h"
#include "../platform.h"

#ifdef USE_OAL

#include <AL/al.h>
#include <AL/alc.h>

namespace xPlatform
{

struct eSource
{
	eSource() : source(0), free_buf(0), first_fill(true)
	{
		memset(buffers, 0, sizeof(buffers));
	}
	void Init()
	{
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
	}
	bool Update(dword data_ready, void* data);
	enum { BUFFER_COUNT = 4 };
	ALuint buffers[BUFFER_COUNT];
	ALuint source;
	ALuint free_buf;
	bool first_fill;
};
bool eSource::Update(dword data_ready, void* data)
{
	if(data_ready < 4096)
		return false;

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
		alBufferData(buffers[free_buf], AL_FORMAT_STEREO16, data, data_ready, 44100);
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
	return next_buf;
}

enum { MAX_SOURCES = 5 };
static eSource sources[MAX_SOURCES];
static ALCdevice* device = NULL;
static ALCcontext* context = NULL;

void InitSound()
{
	device = alcOpenDevice(NULL);
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	alcProcessContext(context);

	int s = Handler()->AudioSources();
	assert(s < MAX_SOURCES);
	for(int i = s; --i >= 0;)
	{
		sources[i].Init();
	}
}

void DoneSound()
{
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		sources[i].Done();
	}
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	context = NULL;
	alcCloseDevice(device);
	device = NULL;
}

void OnLoopSound()
{
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword data_ready = Handler()->AudioDataReady(i);
		void* data = Handler()->AudioData(i);
		if(sources[i].Update(data_ready, data))
			Handler()->AudioDataUse(i, data_ready);
	}
}

}
//namespace xPlatform

#endif//USE_OAL
