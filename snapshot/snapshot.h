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

#ifndef	__SNAPSHOT_H__
#define	__SNAPSHOT_H__

#include "../std.h"

#pragma once

class eSpeccy;

namespace xSnapshot
{
bool Load(eSpeccy* speccy, const char* type, const void* data, size_t data_size);
bool Store(eSpeccy* speccy, const char* file);
}
//namespace xSnapshot

#endif//__SNAPSHOT_H__
