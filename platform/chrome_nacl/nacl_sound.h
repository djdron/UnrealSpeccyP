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

#include "ppapi/cpp/audio.h"
#include <pthread.h>
#include "../../tools/sound_mixer.h"

#pragma once

namespace xPlatform
{

struct eAudio
{
public:
	eAudio() : samples_count(0) { pthread_mutex_init(&mutex, NULL); }
	~eAudio() { pthread_mutex_destroy(&mutex); }

	void Init(pp::Instance* instance);
	void Play() { impl.StartPlayback(); }

	void Update();

protected:
	void GetData(void* samples, uint32_t buffer_size);
	static void _GetData(void* samples, uint32_t buffer_size, void* data) { static_cast<eAudio*>(data)->GetData(samples, buffer_size); }

	pp::Audio impl;
	uint32_t samples_count;
	pthread_mutex_t mutex;
	eSoundMixer mixer;
};

}
//namespace xPlatform

#endif//_CHROME_NACL
