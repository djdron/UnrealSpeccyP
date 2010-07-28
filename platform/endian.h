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

#ifdef USE_BIG_ENDIAN

inline word WordLE(const word* v)	{ return _byteswap_ushort(*v); }
inline void SwapEndian(word* v)		{ *v = _byteswap_ushort(*v); }

#else//USE_BIG_ENDIAN

inline word WordLE(const word* v)	{ return *v; }
inline void SwapEndian(word* v)		{}

#endif//USE_BIG_ENDIAN

#endif//__ENDIAN_H__
