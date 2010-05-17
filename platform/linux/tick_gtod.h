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

#ifndef	__TICK_GTOD_H__
#define	__TICK_GTOD_H__

#include <sys/time.h>

#pragma once

class eTickGtod
{
public:
	eTickGtod() { tick.tv_sec = 0; tick.tv_usec = 0; }
	void	SetCurrent() { gettimeofday(&tick, NULL); }
	eTime	Passed() const
	{
		timeval c;
		gettimeofday(&c, NULL);
		long long int mks = (c.tv_sec - tick.tv_sec)*1e6;
		mks += c.tv_usec - tick.tv_usec;
		eTime t;
		t.SetMks(mks);
		return t;
	}

protected:
	timeval tick;
};

#define TICK_DECLARED
typedef eTickGtod eTick;

#endif//__TICK_GTOD_H__
