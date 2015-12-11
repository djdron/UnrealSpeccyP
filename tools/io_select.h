/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2015 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifndef	__IO_SELECT_H__
#define	__IO_SELECT_H__

#pragma once

namespace xIo
{

class eFileSelect
{
public:
	virtual ~eFileSelect() {}
	virtual bool Valid() const = 0;
	virtual void Next() = 0;
	virtual const char* Name() const = 0;
	virtual bool IsFile() const = 0;
	virtual bool IsDir() const = 0;
};

eFileSelect* FileSelect(const char* path);

bool PathIsRoot(const char* path);

}
//namespace xIo

#endif//__IO_SELECT_H__
