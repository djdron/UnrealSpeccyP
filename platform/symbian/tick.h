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

#ifndef	__TICK_SYMBIAN_H__
#define	__TICK_SYMBIAN_H__

#include <e32std.h>

#pragma once

class eTickSymbian
{
public:
	void	SetCurrent() { tick.UniversalTime(); }
	eTime	Passed() const
	{
		TTime c;
		c.UniversalTime();
		TTimeIntervalMicroSeconds mks = c.MicroSecondsFrom(tick);
		eTime t;
		t.SetMks(mks.Int64());
		return t;
	}
protected:
	TTime tick;
};

#define TICK_DECLARED
typedef eTickSymbian eTick;

#endif//__TICK_SYMBIAN_H__
