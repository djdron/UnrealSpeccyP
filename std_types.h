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

#ifndef __TYPES_H__
#define __TYPES_H__

#pragma once

typedef	unsigned long long int qword;
typedef	unsigned int		dword;
typedef unsigned short		word;
typedef unsigned char		byte;

enum eZeroValue { ZERO };

#ifndef NULL
#define NULL 0
#endif

#define self *this
template<class T> void SAFE_DELETE(T*& p) { if(p) delete p; p = NULL; }
template<class T> void SAFE_DELETE_ARRAY(T*& p) { if(p) delete[] p; p = NULL; }
#define SAFE_CALL(p) if(p) p

#endif//__TYPES_H__
