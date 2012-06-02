/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../../tools/sound_mixer.h"

namespace xPlatform
{

void InitSound()
{
}
void DoneSound()
{
}
static eSoundMixer sound_mixer;
int UpdateSound(byte* buf)
{
	sound_mixer.Update(buf);
	int res = 0;
	dword size = sound_mixer.Ready();
	if(size > (44100*2*2/50)*3)
	{
		res = size;
		sound_mixer.Use(res, buf);
	}
	return res;
}

}
//namespace xPlatform

#endif//_ANDROID
