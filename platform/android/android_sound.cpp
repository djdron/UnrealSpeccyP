/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _ANDROID

#include <oboe/Oboe.h>
#include <oboe/FifoBuffer.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "../../tools/sound_mixer.h"

namespace xPlatform
{

//=============================================================================
//	eOboeAudioEngine
//-----------------------------------------------------------------------------
class eOboeAudioEngine : public oboe::AudioStreamDataCallback, public oboe::AudioStreamErrorCallback
{
public:
	eOboeAudioEngine() : buffer(4, kBufferSizeInFrames)	{ Start(); }
	~eOboeAudioEngine() { Stop(); }

	virtual oboe::DataCallbackResult onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames) override;
	virtual void onErrorAfterClose(oboe::AudioStream* stream, oboe::Result error) override;

	void WriteAudio(eSoundMixer& sm);

private:
	void Start();
	void Stop();
private:
	std::shared_ptr<oboe::AudioStream> stream;
	std::mutex stream_mutex;
	std::mutex buffer_mutex;
	std::condition_variable buffer_cv;
	// each Frame hold 16 bit samples for left+right channels
	// allocating buffer for 3 emulator frames
	static constexpr int32_t kBufferSizeInFrames = 44100*3/50;
	oboe::FifoBuffer buffer;
};
//=============================================================================
//	eOboeAudioEngine::Start
//-----------------------------------------------------------------------------
void eOboeAudioEngine::Start()
{
	std::lock_guard<std::mutex> lock(stream_mutex);
	oboe::AudioStreamBuilder builder;
	builder.setDirection(oboe::Direction::Output)
		->setPerformanceMode(oboe::PerformanceMode::LowLatency)
		->setSharingMode(oboe::SharingMode::Exclusive)
		->setFormat(oboe::AudioFormat::I16)
		->setChannelCount(oboe::ChannelCount::Stereo)
		->setSampleRate(44100)
		->setDataCallback(this)
		->setErrorCallback(this);
	builder.openStream(stream);
	if(stream)
	{
		stream->setBufferSizeInFrames(stream->getFramesPerBurst()*2);
		stream->requestStart();
	}
}
//=============================================================================
//	eOboeAudioEngine::Stop
//-----------------------------------------------------------------------------
void eOboeAudioEngine::Stop()
{
	std::lock_guard<std::mutex> lock(stream_mutex);
	if(stream)
	{
		stream->requestStop();
		stream->close();
		stream.reset();
	}
	buffer_cv.notify_all();
}
//=============================================================================
//	eOboeAudioEngine::WriteAudio
//-----------------------------------------------------------------------------
void eOboeAudioEngine::WriteAudio(eSoundMixer& sm)
{
	std::lock_guard<std::mutex> lock(stream_mutex);
	if(!stream)
		return;
	for(int32_t frames = sm.Ready()/4; frames;)
	{
		auto frames_written = buffer.write(sm.Ptr(), frames);
		if(frames_written > 0)
		{
			frames -= frames_written;
			sm.Use(frames_written*4);
		}
		else
		{
			std::unique_lock<std::mutex> lock(buffer_mutex);
			auto& fifo = buffer;
			bool ok = buffer_cv.wait_for(lock, std::chrono::milliseconds(50), [&fifo]()
			{
				return fifo.getBufferCapacityInFrames() > fifo.getFullFramesAvailable();
			});
			if(!ok)
				break;
		}
	}
}
//=============================================================================
//	eOboeAudioEngine::onAudioReady
//-----------------------------------------------------------------------------
oboe::DataCallbackResult eOboeAudioEngine::onAudioReady(oboe::AudioStream* audioStream, void* audioData, int32_t numFrames)
{
	int32_t framesRead = buffer.read(audioData, numFrames);
	if(framesRead < numFrames)
	{
		memset((byte*)audioData + framesRead*4, 0, (numFrames - framesRead)*4);
	}
	buffer_cv.notify_one();
	return oboe::DataCallbackResult::Continue;
}
//=============================================================================
//	eOboeAudioEngine::onErrorAfterClose
//-----------------------------------------------------------------------------
void eOboeAudioEngine::onErrorAfterClose(oboe::AudioStream* stream, oboe::Result error)
{
	if(error == oboe::Result::ErrorDisconnected)
	{
		std::thread([&]() {
			Stop();
			Start();
		}).detach();
	}
}

static std::unique_ptr<eOboeAudioEngine> oboe_audio;
static eSoundMixer sound_mixer;

//=============================================================================
//	InitSound
//-----------------------------------------------------------------------------
void InitSound()
{
	oboe_audio = std::make_unique<eOboeAudioEngine>();
}
//=============================================================================
//	DoneSound
//-----------------------------------------------------------------------------
void DoneSound()
{
	oboe_audio.reset();
	sound_mixer.Use(sound_mixer.Ready());
}
//=============================================================================
//	UpdateSound
//-----------------------------------------------------------------------------
int UpdateSound(bool skip_data)
{
	if(skip_data)
	{
		for(int s = Handler()->AudioSources(); --s >= 0;)
		{
			Handler()->AudioDataUse(s, Handler()->AudioDataReady(s));
		}
		return 0;
	}
	sound_mixer.Update();
	dword size = sound_mixer.Ready();
	oboe_audio->WriteAudio(sound_mixer);
	return size;
}

}
//namespace xPlatform

#endif//_ANDROID
