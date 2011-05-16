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

#include "../../platform.h"

#ifdef _ANDROID

#include "../../../tools/options.h"
#include "../../../options_common.h"

namespace xPlatform
{

static xOptions::eOption<int>* op_sound = NULL;
static int OpSound() { return op_sound ? *op_sound : (int)S_AY; }

void InitSound()
{
	op_sound = xOptions::eOption<int>::Find("sound");
}
void DoneSound()
{
}
int UpdateSound(byte* buf)
{
	int res = 0;
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		if(i == OpSound() && !Handler()->FullSpeed())
		{
			if(size > 44100*2*2/50*3)//~approx >10600 bytes
			{
				res = size;
				if(res > 32768)
					res = 32768;
				memcpy(buf, Handler()->AudioData(i), res);
				Handler()->AudioDataUse(i, size);
			}
		}
		else
			Handler()->AudioDataUse(i, size);
	}
	return res;
}

}
//namespace xPlatform

#endif//_ANDROID
