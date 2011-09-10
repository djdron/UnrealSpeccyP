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

#ifdef _DINGOO

#include "../platform.h"
#include "../../options_common.h"

#include <dingoo/audio.h>

namespace xPlatform
{

class eAudio
{
public:
	eAudio() : volume(0)
	{
		SetVolume(volume);
		waveout_args wo = { 44100, 16, 2, 30 };
		handle = waveout_open(&wo);
	}
	~eAudio() { waveout_close(handle); }
	void Update();
protected:
	void SetVolume(int v)
	{
		waveout_set_volume(v);
	}
protected:
	void* handle;
	int source;
	int volume;
};

void eAudio::Update()
{
	if(OpVolume() != volume)
	{
		volume = OpVolume();
		SetVolume(volume * 3);
	}
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		bool ui_enabled = Handler()->VideoDataUI();
		if(i == OpSound() && !ui_enabled && !Handler()->FullSpeed())
		{
			waveout_write(handle, (char*)Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
}

void UpdateSound() { static eAudio audio; audio.Update(); }

}
//namespace xPlatform

#endif//_DINGOO
