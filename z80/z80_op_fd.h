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

/* FD prefix opcodes */

#ifndef	__Z80_OP_FD_H__
#define	__Z80_OP_FD_H__

#pragma once

void Opy09() { // add iy,bc
	memptr = iy+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= (((iy & 0x0FFF) + (bc & 0x0FFF)) >> 8) & 0x10; /* HF */
	iy = (iy & 0xFFFF) + (bc & 0xFFFF);
	if (iy & 0x10000) f |= CF;
	f |= (yh & (F5 | F3));
	t += 7;
}
void Opy19() { // add iy,de
	memptr = iy+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= (((iy & 0x0FFF) + (de & 0x0FFF)) >> 8) & 0x10; /* HF */
	iy = (iy & 0xFFFF) + (de & 0xFFFF);
	if (iy & 0x10000) f |= CF;
	f |= (yh & (F5 | F3));
	t += 7;
}
void Opy21() { // ld iy,nnnn
	yl = Read(pc++);
	yh = Read(pc++);
	t += 6;
}
void Opy22() { // ld (nnnn),iy
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	Write(adr, yl);
	Write(adr+1, yh);
	t += 12;
}
void Opy23() { // inc iy
	iy++;
	t += 2;
}
void Opy24() { // inc yh
	inc8(yh);
}
void Opy25() { // dec yh
	dec8(yh);
}
void Opy26() { // ld yh,nn
	yh = Read(pc++);
	t += 3;
}
void Opy29() { // add iy,iy
	memptr = iy+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= ((iy >> 7) & 0x10); /* HF */
	iy = (iy & 0xFFFF)*2;
	if (iy & 0x10000) f |= CF;
	f |= (yh & (F5 | F3));
	t += 7;
}
void Opy2A() { // ld iy,(nnnn)
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	yl = Read(adr);
	yh = Read(adr+1);
	t += 12;
}
void Opy2B() { // dec iy
	iy--;
	t += 2;
}
void Opy2C() { // inc yl
	inc8(yl);
}
void Opy2D() { // dec yl
	dec8(yl);
}
void Opy2E() { // ld yl,nn
	yl = Read(pc++);
	t += 3;
}
void Opy34() { // inc (iy+nn)
	signed char ofs = Read(pc++);
	byte v = Read(iy + ofs);
	inc8(v);
	Write(iy + ofs, v);
	t += 15;
}
void Opy35() { // dec (iy+nn)
	signed char ofs = Read(pc++);
	byte v = Read(iy + ofs);
	dec8(v);
	Write(iy + ofs, v);
	t += 15;
}
void Opy36() { // ld (iy+nn),nn
	signed char ofs = Read(pc++);
	Write(iy + ofs, Read(pc++));
	t += 11;
}
void Opy39() { // add iy,sp
	memptr = iy+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= (((iy & 0x0FFF) + (sp & 0x0FFF)) >> 8) & 0x10; /* HF */
	iy = (iy & 0xFFFF) + (sp & 0xFFFF);
	if (iy & 0x10000) f |= CF;
	f |= (yh & (F5 | F3));
	t += 7;
}
void Opy44() { // ld b,yh
	b = yh;
}
void Opy45() { // ld b,yl
	b = yl;
}
void Opy46() { // ld b,(iy+nn)
	signed char ofs = Read(pc++);
	b = Read(iy + ofs);
	t += 11;
}
void Opy4C() { // ld c,yh
	c = yh;
}
void Opy4D() { // ld c,yl
	c = yl;
}
void Opy4E() { // ld c,(iy+nn)
	signed char ofs = Read(pc++);
	c = Read(iy + ofs);
	t += 11;
}
void Opy54() { // ld d,yh
	d = yh;
}
void Opy55() { // ld d,yl
	d = yl;
}
void Opy56() { // ld d,(iy+nn)
	signed char ofs = Read(pc++);
	d = Read(iy + ofs);
	t += 11;
}
void Opy5C() { // ld e,yh
	e = yh;
}
void Opy5D() { // ld e,yl
	e = yl;
}
void Opy5E() { // ld e,(iy+nn)
	signed char ofs = Read(pc++);
	e = Read(iy + ofs);
	t += 11;
}
void Opy60() { // ld yh,b
	yh = b;
}
void Opy61() { // ld yh,c
	yh = c;
}
void Opy62() { // ld yh,d
	yh = d;
}
void Opy63() { // ld yh,e
	yh = e;
}
void Opy65() { // ld yh,yl
	yh = yl;
}
void Opy66() { // ld h,(iy+nn)
	signed char ofs = Read(pc++);
	h = Read(iy + ofs);
	t += 11;
}
void Opy67() { // ld yh,a
	yh = a;
}
void Opy68() { // ld yl,b
	yl = b;
}
void Opy69() { // ld yl,c
	yl = c;
}
void Opy6A() { // ld yl,d
	yl = d;
}
void Opy6B() { // ld yl,e
	yl = e;
}
void Opy6C() { // ld yl,yh
	yl = yh;
}
void Opy6E() { // ld l,(iy+nn)
	signed char ofs = Read(pc++);
	l = Read(iy + ofs);
	t += 11;
}
void Opy6F() { // ld yl,a
	yl = a;
}
void Opy70() { // ld (iy+nn),b
	signed char ofs = Read(pc++);
	Write(iy + ofs, b);
	t += 11;
}
void Opy71() { // ld (iy+nn),c
	signed char ofs = Read(pc++);
	Write(iy + ofs, c);
	t += 11;
}
void Opy72() { // ld (iy+nn),d
	signed char ofs = Read(pc++);
	Write(iy + ofs, d);
	t += 11;
}
void Opy73() { // ld (iy+nn),e
	signed char ofs = Read(pc++);
	Write(iy + ofs, e);
	t += 11;
}
void Opy74() { // ld (iy+nn),h
	signed char ofs = Read(pc++);
	Write(iy + ofs, h);
	t += 11;
}
void Opy75() { // ld (iy+nn),l
	signed char ofs = Read(pc++);
	Write(iy + ofs, l);
	t += 11;
}
void Opy77() { // ld (iy+nn),a
	signed char ofs = Read(pc++);
	Write(iy + ofs, a);
	t += 11;
}
void Opy7C() { // ld a,yh
	a = yh;
}
void Opy7D() { // ld a,yl
	a = yl;
}
void Opy7E() { // ld a,(iy+nn)
	signed char ofs = Read(pc++);
	a = Read(iy + ofs);
	t += 11;
}
void Opy84() { // add a,yh
	add8(yh);
}
void Opy85() { // add a,yl
	add8(yl);
}
void Opy86() { // add a,(iy+nn)
	signed char ofs = Read(pc++);
	add8(Read(iy + ofs));
	t += 11;
}
void Opy8C() { // adc a,yh
	adc8(yh);
}
void Opy8D() { // adc a,yl
	adc8(yl);
}
void Opy8E() { // adc a,(iy+nn)
	signed char ofs = Read(pc++);
	adc8(Read(iy + ofs));
	t += 11;
}
void Opy94() { // sub yh
	sub8(yh);
}
void Opy95() { // sub yl
	sub8(yl);
}
void Opy96() { // sub (iy+nn)
	signed char ofs = Read(pc++);
	sub8(Read(iy + ofs));
	t += 11;
}
void Opy9C() { // sbc a,yh
	sbc8(yh);
}
void Opy9D() { // sbc a,yl
	sbc8(yl);
}
void Opy9E() { // sbc a,(iy+nn)
	signed char ofs = Read(pc++);
	sbc8(Read(iy + ofs));
	t += 11;
}
void OpyA4() { // and yh
	and8(yh);
}
void OpyA5() { // and yl
	and8(yl);
}
void OpyA6() { // and (iy+nn)
	signed char ofs = Read(pc++);
	and8(Read(iy + ofs));
	t += 11;
}
void OpyAC() { // xor yh
	xor8(yh);
}
void OpyAD() { // xor yl
	xor8(yl);
}
void OpyAE() { // xor (iy+nn)
	signed char ofs = Read(pc++);
	xor8(Read(iy + ofs));
	t += 11;
}
void OpyB4() { // or yh
	or8(yh);
}
void OpyB5() { // or yl
	or8(yl);
}
void OpyB6() { // or (iy+nn)
	signed char ofs = Read(pc++);
	or8(Read(iy + ofs));
	t += 11;
}
void OpyBC() { // cp yh
	cp8(yh);
}
void OpyBD() { // cp yl
	cp8(yl);
}
void OpyBE() { // cp (iy+nn)
	signed char ofs = Read(pc++);
	cp8(Read(iy + ofs));
	t += 11;
}
void OpyE1() { // pop iy
	yl = Read(sp++);
	yh = Read(sp++);
	t += 6;
}
void OpyE3() { // ex (sp),iy
	unsigned tmp = Read(sp) + 0x100*Read(sp + 1);
	Write(sp, yl);
	Write(sp+1, yh);
	memptr = tmp;
	iy = tmp;
	t += 15;
}
void OpyE5() { // push iy
	Write(--sp, yh);
	Write(--sp, yl);
	t += 7;
}
void OpyE9() { // jp (iy)
	pc = iy;
}
void OpyF9() { // ld sp,iy
	sp = iy;
	t += 2;
}

#endif//__Z80_OP_FD_H__
