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

#ifndef	__IO_H__
#define	__IO_H__

#pragma once

namespace xIo
{

enum { MAX_PATH_LEN = 1024};

void SetResourcePath(const char* resource_path);
const char* ResourcePath(const char* path);

void SetProfilePath(const char* profile_path);
const char* ProfilePath(const char* path);

}
//namespace xIo

#endif//__IO_H__
