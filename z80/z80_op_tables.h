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

#ifndef	__Z80_TABLES_H__
#define	__Z80_TABLES_H__

#pragma once

namespace xZ80
{

extern const word* daatab;
extern const byte* incf;
extern const byte* decf;
extern const byte* rlcf;
extern const byte* rrcf;
extern const byte* rl0;
extern const byte* rl1;
extern const byte* rr0;
extern const byte* rr1;
extern const byte* sraf;

extern const byte* adcf;	// flags for adc and add
extern const byte* sbcf;	// flags for sub and sbc
extern const byte* cpf;		// flags for cp
extern const byte* cpf8b;	// flags for CPD/CPI/CPDR/CPIR
extern const byte* log_f;
extern const byte* rlcaf;
extern const byte* rrcaf;
extern const byte* rol;
extern const byte* ror;

}//namespace xZ80

#endif//__Z80_TABLES_H__
