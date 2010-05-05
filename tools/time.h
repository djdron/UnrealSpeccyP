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

#ifndef	__TIME_H__
#define	__TIME_H__

#pragma once

class eTime
{
public:
	eTime() : mks(0) {}

	eTime(float sec) { mks = sec*1e6f; }

	float	Sec() const	{ return float(mks)/1e6f; }
	float	Ms() const	{ return float(mks)/1e3f; }
	qword	Mks() const	{ return mks; }

	void	SetMks(qword _mks) { mks = _mks; }

	eTime&	operator+=(const eTime& t) { mks += t.mks; return self; }
	eTime&	operator-=(const eTime& t) { mks -= t.mks; return self; }
	bool	operator<(const eTime& t) const { return mks < t.mks; }
	bool	operator>(const eTime& t) const { return mks > t.mks; }

protected:
	qword	mks;
};

#endif//__TIME_H__
