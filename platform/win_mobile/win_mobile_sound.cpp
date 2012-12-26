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

#ifdef _WIN_MOBILE

#include <windows.h>
#include "../../tools/log.h"
#include "../../options_common.h"

namespace xPlatform
{

class eAudio
{
public:
	eAudio() : handle(NULL)
	{
		WAVEFORMATEX wf;
		wf.cbSize = sizeof(wf);
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.nSamplesPerSec = 44100;
		wf.wBitsPerSample = 16;
		wf.nChannels = 2;
		wf.nBlockAlign = 4;
		wf.nAvgBytesPerSec = 176400;
		MMRESULT r = waveOutOpen(&handle, WAVE_MAPPER, &wf, NULL, 0, CALLBACK_NULL);
		if(r != MMSYSERR_NOERROR)
		{
			_LOG("WinMobile: audio init failed");
		}
	}
	~eAudio()
	{
		if(handle)
			waveOutClose(handle);
	}
	void Update();
protected:
	HWAVEOUT handle;
};

void eAudio::Update()
{
	if(!handle)
		return;
	using namespace xPlatform;
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		bool ui_enabled = Handler()->VideoDataUI() != NULL;
		if(i == *OPTION_GET(op_sound_source) && !ui_enabled && !Handler()->FullSpeed())
		{
			WAVEHDR whdr;
			whdr.dwBufferLength = size;
			whdr.lpData = (LPSTR)Handler()->AudioData(i);
			whdr.dwFlags = 0;
			MMRESULT r = waveOutPrepareHeader(handle, &whdr, sizeof(whdr));
			if(r == MMSYSERR_NOERROR)
				waveOutWrite(handle, &whdr, sizeof(whdr));
			while(!(whdr.dwFlags&WHDR_DONE))
				Sleep(1);
			waveOutUnprepareHeader(handle, &whdr, sizeof(whdr));
		}
		Handler()->AudioDataUse(i, size);
	}
}
void OnLoopSound()
{
	static eAudio audio;
	audio.Update();
}

}
//namespace xPlatform

#endif//_WIN_MOBILE
