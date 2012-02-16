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

#ifndef	__ENDIAN_H__
#define	__ENDIAN_H__

#pragma once

//#define USE_BIG_ENDIAN

inline word swap_byte_order(const word& v) { return (v >> 8) | (v << 8); }

#ifdef USE_BIG_ENDIAN

inline word SwapWord(const word* v)		{ return swap_byte_order(*v); }
inline void SwapEndian(word* v)			{ *v = swap_byte_order(*v); }

#else//USE_BIG_ENDIAN

inline word SwapWord(const word* v)		{ return *v; }
inline void SwapEndian(word* v)			{}

#endif//USE_BIG_ENDIAN

static inline word Word(const byte* ptr)	{ return ptr[0] | word(ptr[1]) << 8; }
static inline word WordBE(const byte* ptr)	{ return word(ptr[0]) << 8 | ptr[1]; }
static inline dword Dword(const byte* ptr)
{
	return ptr[0] | dword(ptr[1]) << 8 | dword(ptr[2]) << 16 | dword(ptr[3]) << 24;
}

#endif//__ENDIAN_H__
