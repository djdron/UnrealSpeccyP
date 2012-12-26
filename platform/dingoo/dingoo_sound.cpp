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

OPTION_USING(eOptionB, op_ay);

namespace xPlatform
{

static class eAudio : public xOptions::eRootOption<xOptions::eOptionB>
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
	virtual const char* Name() const { return "sound"; }
	virtual int Order() const { return 25; }
protected:
	void SetVolume(int v) { waveout_set_volume(v); }
	virtual void OnOption()
	{
		Option(OPTION_GET(op_sound_source));
		Option(OPTION_GET(op_volume));
		Option(OPTION_GET(op_ay));
	}
protected:
	void* handle;
	int source;
	int volume;
} audio;

void eAudio::Update()
{
	bool ui_enabled = Handler()->VideoDataUI();
	int op_volume = *OPTION_GET(op_volume);
	if((!ui_enabled && op_volume != volume) || (ui_enabled && volume))
	{
		volume = !ui_enabled ? op_volume : 0;
		SetVolume(volume * 3);
	}
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		if(i == *OPTION_GET(op_sound_source) && !Handler()->FullSpeed())
		{
			waveout_write(handle, (char*)Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
}

void UpdateAudio() { audio.Update(); }

}
//namespace xPlatform

#endif//_DINGOO
