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

#ifdef USE_QT

#include "qt_sound.h"

qint64 eAudioStream::readData(char* data, qint64 maxlen)
{
	if(maxlen > BUF_SIZE)
		maxlen = BUF_SIZE;
	size_t use = ready < (size_t)maxlen ? ready : (size_t)maxlen;
	Use(data, use);
	return use;
}
void eAudioStream::Fill(void* data, size_t size)
{
	if(ready + size < BUF_SIZE)
	{
		memcpy(buffer + ready, data, size);
		ready += size;
	}
	else
		ready = 0;
}
void eAudioStream::Use(void* out, size_t size)
{
	if(size)
	{
		memcpy(out, buffer, size);
		if(ready > size)
			memmove(buffer, buffer + size, ready - size);
		ready -= size;
	}
}

#endif//USE_QT
