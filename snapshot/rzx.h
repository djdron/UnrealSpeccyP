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

#ifndef __RZX_H__
#define __RZX_H__

#include "../std_types.h"

class eRZX
{
public:
	enum eError { OK, NOTFOUND, INVALID, FINISHED, UNSUPPORTED, NOMEMORY, SYNCLOST };

	eRZX();
	~eRZX();

	class eHandler
	{
	public:
		virtual bool RZX_OnOpenSnapshot(const char* name, const void* data, size_t data_size) = 0;
	};

	eError Open(const void* data, size_t data_size, eHandler* handler);
	eError Update(int* icount);
	eError IoRead(byte* data);

private:
	class eImpl;
	eImpl* impl;
};

#endif//__RZX_H__
