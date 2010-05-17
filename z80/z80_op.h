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

#ifndef	__Z80_OP_H__
#define	__Z80_OP_H__

#pragma once

void inc8(byte& x)
{
	f = incf[x] | (f & CF);
	x++;
}
void dec8(byte& x)
{
	f = decf[x] | (f & CF);
	x--;
}
void add8(byte src)
{
	f = adcf[a + src*0x100];
	a += src;
}
void adc8(byte src)
{
	byte carry = f & CF;
	f = adcf[a + src*0x100 + 0x10000*carry];
	a += src + carry;
}
void sub8(byte src)
{
	f = sbcf[a*0x100 + src];
	a -= src;
}
void sbc8(byte src)
{
	byte carry = f & CF;
	f = sbcf[a*0x100 + src + 0x10000*carry];
	a -= src + carry;
}
void and8(byte src)
{
	a &= src;
	f = log_f[a] | HF;
}
void or8(byte src)
{
	a |= src;
	f = log_f[a];
}
void xor8(byte src)
{
	a ^= src;
	f = log_f[a];
}
void cp8(byte src)
{
	f = cpf[a*0x100 + src];
}
void bit(byte src, byte bit)
{
	f = log_f[src & (1 << bit)] | HF | (f & CF) | (src & (F3|F5));
}
void bitmem(byte src, byte bit)
{
	f = log_f[src & (1 << bit)] | HF | (f & CF);
	f = (f & ~(F3|F5)) | (mem_h & (F3|F5));
}
void res(byte& src, byte bit) const
{
	src &= ~(1 << bit);
}
byte resbyte(byte src, byte bit) const
{
	return src & ~(1 << bit);
}
void set(byte& src, byte bit) const
{
	src |= (1 << bit);
}
byte setbyte(byte src, byte bit) const
{
	return src | (1 << bit);
}

#endif//__Z80_OP_H__
