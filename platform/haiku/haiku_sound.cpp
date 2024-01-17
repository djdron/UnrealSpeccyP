/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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

#include <Locker.h>
#include <SoundPlayer.h>

#include "../platform.h"
#include "../../options_common.h"
#include "../../tools/options.h"
#include "../../tools/sound_mixer.h"

namespace xPlatform
{
#define HAIKU_SOUND_BUFFER_SIZE (3528 * 4)

static BSoundPlayer *sound_player;
static BLocker sound_locker("sound_locker");
static eSoundMixer sound_mixer;

static void AudioCallback(void *cookie, void *buf, size_t length, const media_raw_audio_format &format)
{
	sound_locker.Lock();

	if(length <= sound_mixer.Ready()) {
		memcpy(buf, sound_mixer.Ptr(), length);
		sound_mixer.Use(length);
	} else {
		memcpy(buf, sound_mixer.Ptr(), sound_mixer.Ready());
		memset((byte*)buf + sound_mixer.Ready(), 0, length - sound_mixer.Ready());
		sound_mixer.Use(sound_mixer.Ready());
	}
	
	sound_locker.Unlock();
}

void InitAudio()
{
	using namespace xOptions;
	struct eOptionBX : public eOptionB
	{
		void Unuse() { customizable = false; storeable = false; }
	};
	eOptionBX* o = (eOptionBX*)eOptionB::Find("sound");
	SAFE_CALL(o)->Unuse();

	media_raw_audio_format spec = {
		44100,
		2,
		media_raw_audio_format::B_AUDIO_SHORT,
		B_MEDIA_LITTLE_ENDIAN,
		HAIKU_SOUND_BUFFER_SIZE
	};

	sound_player = new BSoundPlayer(&spec, "USP", AudioCallback);
	sound_player->Start();
	sound_player->SetHasData(true);
}

void VolumeAudio(float volume)
{
	float db = 20 * log(volume / 10.0);
	sound_player->SetVolumeDB(db);
}

void UpdateAudio()
{
	sound_locker.Lock();
	sound_mixer.Update();

	static bool audio_filled = false;
	bool audio_filled_new = sound_mixer.Ready() > HAIKU_SOUND_BUFFER_SIZE;
	if(audio_filled != audio_filled_new)
	{
		audio_filled = audio_filled_new;
		Handler()->VideoPaused(audio_filled);
	}
	
	sound_locker.Unlock();
}

void DoneAudio()
{
	sound_player->SetHasData(false);
	delete sound_player;
}

}
