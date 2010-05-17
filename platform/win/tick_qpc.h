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

#ifndef	__TICK_QPC_H__
#define	__TICK_QPC_H__

#include <windows.h>

#pragma once

class eTickQpc
{
public:
	eTickQpc() { tick.QuadPart = 0; }
	void	SetCurrent() { QueryPerformanceCounter(&tick); }
	eTime	Passed() const
	{
		static LARGE_INTEGER f = QpcFreq();
		LARGE_INTEGER c;
		QueryPerformanceCounter(&c);
		c.QuadPart -= tick.QuadPart;
		c.QuadPart *= 1e6;
		c.QuadPart /= f.QuadPart;
		eTime t;
		t.SetMks(c.QuadPart);
		return t;
	}

protected:
	LARGE_INTEGER QpcFreq() const
	{
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);
		return f;
	}
	LARGE_INTEGER tick;
};

#define TICK_DECLARED
typedef eTickQpc eTick;

#endif//__TICK_QPC_H__
