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

/* not prefixed opcodes */

#ifndef	__Z80_OP_NOPREFIX_H__
#define	__Z80_OP_NOPREFIX_H__

#pragma once

void Op00() { // nop
	/* note: don't inc t: 4 cycles already wasted in m1_cycle() */
}
void Op01() { // ld bc,nnnn
	c = Read(pc++);
	b = Read(pc++);
	t += 6;
}
void Op02() { // ld (bc),a
	mem_h = a;
	t += 3;
	Write(bc, a); //Alone Coder
}
void Op03() { // inc bc
	bc++;
	t += 2;
}
void Op04() { // inc b
	inc8(b);
}
void Op05() { // dec b
	dec8(b);
}
void Op06() { // ld b,nn
	b = Read(pc++);
	t += 3;
}
void Op07() { // rlca
	f = rlcaf[a] | (f & (SF | ZF | PV));
	a = rol[a];
}
void Op08() { // ex af,af'
	unsigned tmp = af;
	af = alt.af;
	alt.af = tmp;
}
void Op09() { // add hl,bc
	memptr = hl+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= (((hl & 0x0FFF) + (bc & 0x0FFF)) >> 8) & 0x10; /* HF */
	hl = (hl & 0xFFFF) + (bc & 0xFFFF);
	if (hl & 0x10000) f |= CF;
	f |= (h & (F5 | F3));
	t += 7;
}
void Op0A() { // ld a,(bc)
	memptr = bc+1;
	a = Read(bc);
	t += 3;
}
void Op0B() { // dec bc
	bc--;
	t += 2;
}
void Op0C() { // inc c
	inc8(c);
}
void Op0D() { // dec c
	dec8(c);
}
void Op0E() { // ld c,nn
	c = Read(pc++);
	t += 3;
}
void Op0F() { // rrca
	f = rrcaf[a] | (f & (SF | ZF | PV));
	a = ror[a];
}
void Op10() { // djnz rr
	if (--b) {
		signed char offs = (char)Read(pc);
		memptr = pc += offs+1, t += 9;
	} else pc++, t += 4;
}
void Op11() { // ld de,nnnn
	e = Read(pc++);
	d = Read(pc++);
	t += 6;
}
void Op12() { // ld (de),a
	mem_h = a;
	t += 3;
	Write(de, a); //Alone Coder
}
void Op13() { // inc de
	de++;
	t += 2;
}
void Op14() { // inc d
	inc8(d);
}
void Op15() { // dec d
	dec8(d);
}
void Op16() { // ld d,nn
	d = Read(pc++);
	t += 3;
}
void Op17() { // rla
	byte new_a = (a << 1) + (f & 1);
	f = rlcaf[a] | (f & (SF | ZF | PV)); // use same table with rlca
	a = new_a;
}
void Op18() { // jr rr
	signed char offs = (char)Read(pc);
	pc += offs+1;
	memptr = pc;
	t += 8;
}
void Op19() { // add hl,de
	memptr = hl+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= (((hl & 0x0FFF) + (de & 0x0FFF)) >> 8) & 0x10; /* HF */
	hl = (hl & 0xFFFF) + (de & 0xFFFF);
	if (hl & 0x10000) f |= CF;
	f |= (h & (F5 | F3));
	t += 7;
}
void Op1A() { // ld a,(de)
	memptr = de+1;
	a = Read(de);
	t += 3;
}
void Op1B() { // dec de
	de--;
	t += 2;
}
void Op1C() { // inc e
	inc8(e);
}
void Op1D() { // dec e
	dec8(e);
}
void Op1E() { // ld e,nn
	e = Read(pc++);
	t += 3;
}
void Op1F() { // rra
	byte new_a = (a >> 1) + (f << 7);
	f = rrcaf[a] | (f & (SF | ZF | PV)); // use same table with rrca
	a = new_a;
}
void Op20() { // jr nz, rr
	if (!(f & ZF)) {
		signed char offs = (char)Read(pc);
		memptr = pc += offs+1, t += 8;
	} else pc++, t += 3;
}
void Op21() { // ld hl,nnnn
	l = Read(pc++);
	h = Read(pc++);
	t += 6;
}
void Op22() { // ld (nnnn),hl
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	t += 9;
	Write(adr, l);
	t += 3;
	Write(adr+1, h);
}
void Op23() { // inc hl
	hl++;
	t += 2;
}
void Op24() { // inc h
	inc8(h);
}
void Op25() { // dec h
	dec8(h);
}
void Op26() { // ld h,nn
	h = Read(pc++);
	t += 3;
}
void Op27()
{ // daa
	af = daatab[a + 0x100*((f & 3) + ((f >> 2) & 4))];
}
void Op28() { // jr z,rr
	if ((f & ZF)) {
		signed char offs = (char)Read(pc);
		memptr = pc += offs+1, t += 8;
	} else pc++, t += 3;
}
void Op29() { // add hl,hl
	memptr = hl+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= ((hl >> 7) & 0x10); /* HF */
	hl = (hl & 0xFFFF)*2;
	if (hl & 0x10000) f |= CF;
	f |= (h & (F5 | F3));
	t += 7;
}
void Op2A() { // ld hl,(nnnn)
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	l = Read(adr);
	h = Read(adr+1);
	t += 12;
}
void Op2B() { // dec hl
	hl--;
	t += 2;
}
void Op2C() { // inc l
	inc8(l);
}
void Op2D() { // dec l
	dec8(l);
}
void Op2E() { // ld l,nn
	l = Read(pc++);
	t += 3;
}
void Op2F() { // cpl
	a ^= 0xFF;
	f = (f & ~(F3|F5)) | NF | HF | (a & (F3|F5));
}
void Op30() { // jr nc, rr
	if (!(f & CF)) {
		signed char offs = (char)Read(pc);
		memptr = pc += offs+1, t += 8;
	} else pc++, t += 3;
}
void Op31() { // ld sp,nnnn
	sp_l = Read(pc++);
	sp_h = Read(pc++);
	t += 6;
}
void Op32() { // ld (nnnn),a
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = ((adr+1) & 0xFF) + (a << 8);
	mem_h = a;
	t += 9;
	Write(adr, a);
}
void Op33() { // inc sp
	sp++;
	t += 2;
}
void Op34() { // inc (hl)
	byte v = Read(hl);
	inc8(v);
	t += 7;
	Write(hl, v);
}
void Op35() { // dec (hl)
	byte v = Read(hl);
	dec8(v);
	t += 7;
	Write(hl, v);
}
void Op36() { // ld (hl),nn
	t += 6;
	Write(hl, Read(pc++));
}
void Op37() { // scf
	f = (f & ~(HF|NF)) | (a & (F3|F5)) | CF;
}
void Op38() { // jr c,rr
	if ((f & CF)) {
		signed char offs = (char)Read(pc);
		memptr = pc += offs+1, t += 8;
	} else pc++, t += 3;
}
void Op39() { // add hl,sp
	memptr = hl+1;
	f = (f & ~(NF | CF | F5 | F3 | HF));
	f |= (((hl & 0x0FFF) + (sp & 0x0FFF)) >> 8) & 0x10; /* HF */
	hl = (hl & 0xFFFF) + (sp & 0xFFFF);
	if (hl & 0x10000) f |= CF;
	f |= (h & (F5 | F3));
	t += 7;
}
void Op3A() { // ld a,(nnnn)
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	a = Read(adr);
	t += 9;
}
void Op3B() { // dec sp
	sp--;
	t += 2;
}
void Op3C() { // inc a
	inc8(a);
}
void Op3D() { // dec a
	dec8(a);
}
void Op3E() { // ld a,nn
	a = Read(pc++);
	t += 3;
}
void Op3F() { // ccf
	f = ((f & ~(NF|HF)) | ((f << 4) & HF) | (a & (F3|F5))) ^ CF;
}
void Op40() {} // ld b,b
void Op49() {} // ld c,c
void Op52() {} // ld d,d
void Op5B() {} // ld e,e
void Op64() {} // ld h,h
void Op6D() {} // ld l,l
void Op7F() {} // ld a,a
void Op41() { // ld b,c
	b = c;
}
void Op42() { // ld b,d
	b = d;
}
void Op43() { // ld b,e
	b = e;
}
void Op44() { // ld b,h
	b = h;
}
void Op45() { // ld b,l
	b = l;
}
void Op46() { // ld b,(hl)
	b = Read(hl);
	t += 3;
}
void Op47() { // ld b,a
	b = a;
}
void Op48() { // ld c,b
	c = b;
}
void Op4A() { // ld c,d
	c = d;
}
void Op4B() { // ld c,e
	c = e;
}
void Op4C() { // ld c,h
	c = h;
}
void Op4D() { // ld c,l
	c = l;
}
void Op4E() { // ld c,(hl)
	c = Read(hl);
	t += 3;
}
void Op4F() { // ld c,a
	c = a;
}
void Op50() { // ld d,b
	d = b;
}
void Op51() { // ld d,c
	d = c;
}
void Op53() { // ld d,e
	d = e;
}
void Op54() { // ld d,h
	d = h;
}
void Op55() { // ld d,l
	d = l;
}
void Op56() { // ld d,(hl)
	d = Read(hl);
	t += 3;
}
void Op57() { // ld d,a
	d = a;
}
void Op58() { // ld e,b
	e = b;
}
void Op59() { // ld e,c
	e = c;
}
void Op5A() { // ld e,d
	e = d;
}
void Op5C() { // ld e,h
	e = h;
}
void Op5D() { // ld e,l
	e = l;
}
void Op5E() { // ld e,(hl)
	e = Read(hl);
	t += 3;
}
void Op5F() { // ld e,a
	e = a;
}
void Op60() { // ld h,b
	h = b;
}
void Op61() { // ld h,c
	h = c;
}
void Op62() { // ld h,d
	h = d;
}
void Op63() { // ld h,e
	h = e;
}
void Op65() { // ld h,l
	h = l;
}
void Op66() { // ld h,(hl)
	h = Read(hl);
	t += 3;
}
void Op67() { // ld h,a
	h = a;
}
void Op68() { // ld l,b
	l = b;
}
void Op69() { // ld l,c
	l = c;
}
void Op6A() { // ld l,d
	l = d;
}
void Op6B() { // ld l,e
	l = e;
}
void Op6C() { // ld l,h
	l = h;
}
void Op6E() { // ld l,(hl)
	l = Read(hl);
	t += 3;
}
void Op6F() { // ld l,a
	l = a;
}
void Op70() { // ld (hl),b
	t += 3;
	Write(hl, b);
}
void Op71() { // ld (hl),c
	t += 3;
	Write(hl, c);
}
void Op72() { // ld (hl),d
	t += 3;
	Write(hl, d);
}
void Op73() { // ld (hl),e
	t += 3;
	Write(hl, e);
}
void Op74() { // ld (hl),h
	t += 3;
	Write(hl, h);
}
void Op75() { // ld (hl),l
	t += 3;
	Write(hl, l);
}
void Op76() { // halt
	halted = 1;
	haltpos = t;
	unsigned int st = (frame_tacts - t-1)/4+1;
	t += 4*st;
	r_low += st;
	fetches += st;
}
void Op77() { // ld (hl),a
	t += 3;
	Write(hl, a);
}
void Op78() { // ld a,b
	a = b;
}
void Op79() { // ld a,c
	a = c;
}
void Op7A() { // ld a,d
	a = d;
}
void Op7B() { // ld a,e
	a = e;
}
void Op7C() { // ld a,h
	a = h;
}
void Op7D() { // ld a,l
	a = l;
}
void Op7E() { // ld a,(hl)
	a = Read(hl);
	t += 3;
}
void Op80() { // add a,b
	add8(b);
}
void Op81() { // add a,c
	add8(c);
}
void Op82() { // add a,d
	add8(d);
}
void Op83() { // add a,e
	add8(e);
}
void Op84() { // add a,h
	add8(h);
}
void Op85() { // add a,l
	add8(l);
}
void Op86() { // add a,(hl)
	add8(Read(hl));
	t += 3;
}
void Op87() { // add a,a
	add8(a);
}
void Op88() { // adc a,b
	adc8(b);
}
void Op89() { // adc a,c
	adc8(c);
}
void Op8A() { // adc a,d
	adc8(d);
}
void Op8B() { // adc a,e
	adc8(e);
}
void Op8C() { // adc a,h
	adc8(h);
}
void Op8D() { // adc a,l
	adc8(l);
}
void Op8E() { // adc a,(hl)
	adc8(Read(hl));
	t += 3;
}
void Op8F() { // adc a,a
	adc8(a);
}
void Op90() { // sub b
	sub8(b);
}
void Op91() { // sub c
	sub8(c);
}
void Op92() { // sub d
	sub8(d);
}
void Op93() { // sub e
	sub8(e);
}
void Op94() { // sub h
	sub8(h);
}
void Op95() { // sub l
	sub8(l);
}
void Op96() { // sub (hl)
	sub8(Read(hl));
	t += 3;
}
void Op97() { // sub a
	af = ZF | NF;
}
void Op98() { // sbc a,b
	sbc8(b);
}
void Op99() { // sbc a,c
	sbc8(c);
}
void Op9A() { // sbc a,d
	sbc8(d);
}
void Op9B() { // sbc a,e
	sbc8(e);
}
void Op9C() { // sbc a,h
	sbc8(h);
}
void Op9D() { // sbc a,l
	sbc8(l);
}
void Op9E() { // sbc a,(hl)
	sbc8(Read(hl));
	t += 3;
}
void Op9F() { // sbc a,a
	sbc8(a);
}
void OpA0() { // and b
	and8(b);
}
void OpA1() { // and c
	and8(c);
}
void OpA2() { // and d
	and8(d);
}
void OpA3() { // and e
	and8(e);
}
void OpA4() { // and h
	and8(h);
}
void OpA5() { // and l
	and8(l);
}
void OpA6() { // and (hl)
	and8(Read(hl));
	t += 3;
}
void OpA7() { // and a
	and8(a); // already optimized by compiler
}
void OpA8() { // xor b
	xor8(b);
}
void OpA9() { // xor c
	xor8(c);
}
void OpAA() { // xor d
	xor8(d);
}
void OpAB() { // xor e
	xor8(e);
}
void OpAC() { // xor h
	xor8(h);
}
void OpAD() { // xor l
	xor8(l);
}
void OpAE() { // xor (hl)
	xor8(Read(hl));
	t += 3;
}
void OpAF() { // xor a
	af = ZF | PV;
}
void OpB0() { // or b
	or8(b);
}
void OpB1() { // or c
	or8(c);
}
void OpB2() { // or d
	or8(d);
}
void OpB3() { // or e
	or8(e);
}
void OpB4() { // or h
	or8(h);
}
void OpB5() { // or l
	or8(l);
}
void OpB6() { // or (hl)
	or8(Read(hl));
	t += 3;
}
void OpB7() { // or a
	or8(a);  // already optimized by compiler
}
void OpB8() { // cp b
	cp8(b);
}
void OpB9() { // cp c
	cp8(c);
}
void OpBA() { // cp d
	cp8(d);
}
void OpBB() { // cp e
	cp8(e);
}
void OpBC() { // cp h
	cp8(h);
}
void OpBD() { // cp l
	cp8(l);
}
void OpBE() { // cp (hl)
	cp8(Read(hl));
	t += 3;
}
void OpBF() { // cp a
	cp8(a); // can't optimize: F3,F5 depends on A
}
void OpC0() { // ret nz
	if (!(f & ZF)) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpC1() { // pop bc
	c = Read(sp++);
	b = Read(sp++);
	t += 6;
}
void OpC2() { // jp nz,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (!(f & ZF)) {
		pc = addr;
	} else pc += 2;
}
void OpC3() { // jp nnnn
	unsigned lo = Read(pc++);
	pc = lo + 0x100*Read(pc);
	memptr = pc;
	t += 6;
}
void OpC4() { // call nz,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (!(f & ZF)) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpC5() { // push bc
	t += 7;
	Write(--sp, b);
	Write(--sp, c);
}
void OpC6() { // add a,nn
	add8(Read(pc++));
	t += 3;
}
void OpC7() { // rst 00
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x00;
	memptr = 0x00;
	t += 7;
}
void OpC8() { // ret z
	if (f & ZF) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpC9() { // ret
	unsigned addr = Read(sp++);
	addr += 0x100*Read(sp++);
	pc = addr;
	memptr = addr;
	t += 6;
}
void OpCA() { // jp z,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if(f & ZF)
	{
		pc = addr;
	}
	else
		pc += 2;
}
void OpCC() { // call z,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (f & ZF) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpCD() { // call
	unsigned addr = Read(pc++);
	addr += 0x100*Read(pc++);
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = addr;
	memptr = addr;
	t += 13;
}
void OpCE() { // adc a,nn
	adc8(Read(pc++));
	t += 3;
}
void OpCF() { // rst 08
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x08;
	memptr = 0x08;
	mem_h = 0;
	t += 7;
}
void OpD0() { // ret nc
	if (!(f & CF)) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpD1() { // pop de
	e = Read(sp++);
	d = Read(sp++);
	t += 6;
}
void OpD2() { // jp nc,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (!(f & CF)) {
		pc = addr;
	} else pc += 2;
}
void OpD3() { // out (nn),a
	unsigned port = Read(pc++);
	t += 7;
	memptr = ((port+1) & 0xFF) + (a << 8);
	IoWrite(port + (a << 8), a);
}
void OpD4() { // call nc,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (!(f & CF)) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpD5() { // push de
	t += 7;
	Write(--sp, d);
	Write(--sp, e);
}
void OpD6() { // sub nn
	sub8(Read(pc++));
	t += 3;
}
void OpD7() { // rst 10
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x10;
	memptr = 0x10;
	t += 7;
}
void OpD8() { // ret c
	if (f & CF) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpD9() { // exx
	unsigned tmp = bc; bc = alt.bc; alt.bc = tmp;
	tmp = de; de = alt.de; alt.de = tmp;
	tmp = hl; hl = alt.hl; alt.hl = tmp;
}
void OpDA() { // jp c,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (f & CF) {
		pc = addr;
	} else pc += 2;
}
void OpDB() { // in a,(nn)
	unsigned port = Read(pc++) + (a << 8);
	memptr = (a << 8) + port+1;
	t += 7;
	a = IoRead(port);
}
void OpDC() { // call c,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (f & CF) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpDE() { // sbc a,nn
	sbc8(Read(pc++));
	t += 3;
}
void OpDF() { // rst 18
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x18;
	memptr = 0x18;
	t += 7;
}
void OpE0() { // ret po
	if (!(f & PV)) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpE1() { // pop hl
	l = Read(sp++);
	h = Read(sp++);
	t += 6;
}
void OpE2() { // jp po,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (!(f & PV)) {
		pc = addr;
	} else pc += 2;
}
void OpE3() { // ex (sp),hl
	unsigned tmp = Read(sp) + 0x100*Read(sp + 1);
	Write(sp, l);
	Write(sp+1, h);
	memptr = tmp;
	hl = tmp;
	t += 15;
}
void OpE4() { // call po,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (!(f & PV)) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpE5() { // push hl
	t += 7;
	Write(--sp, h);
	Write(--sp, l);
}
void OpE6() { // and nn
	and8(Read(pc++));
	t += 3;
}
void OpE7() { // rst 20
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x20;
	memptr = 0x20;
	t += 7;
}
void OpE8() { // ret pe
	if (f & PV) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpE9() { // jp (hl)
	pc = hl;
}
void OpEA() { // jp pe,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (f & PV) {
		pc = addr;
	} else pc += 2;
}
void OpEB() { // ex de,hl
	unsigned tmp = de;
	de = hl;
	hl = tmp;
}
void OpEC() { // call pe,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (f & PV) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpEE() { // xor nn
	xor8(Read(pc++));
	t += 3;
}
void OpEF() { // rst 28
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x28;
	memptr = 0x28;
	t += 7;
}
void OpF0() { // ret p
	if (!(f & SF)) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpF1() { // pop af
	f = Read(sp++);
	a = Read(sp++);
	t += 6;
}
void OpF2() { // jp p,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (!(f & SF)) {
		pc = addr;
	} else pc += 2;
}
void OpF3() { // di
	iff1 = iff2 = 0;
}
void OpF4() { // call p,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (!(f & SF)) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpF5() { // push af
	t += 7;
	Write(--sp, a);
	Write(--sp, f);
}
void OpF6() { // or nn
	or8(Read(pc++));
	t += 3;
}
void OpF7() { // rst 30
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x30;
	memptr = 0x30;
	t += 7;
}
void OpF8() { // ret m
	if (f & SF) {
		unsigned addr = Read(sp++);
		addr += 0x100*Read(sp++);
		pc = addr;
		memptr = addr;
		t += 7;
	} else t += 1;
}
void OpF9() { // ld sp,hl
	sp = hl;
	t += 2;
}
void OpFA() { // jp m,nnnn
	t += 6;
	unsigned addr = Read(pc);
	addr += 0x100*Read(pc+1);
	memptr = addr;
	if (f & SF) {
		pc = addr;
	} else pc += 2;
}
void OpFB() { // ei
	iff1 = iff2 = 1;
	eipos = t;
}
void OpFC() { // call m,nnnn
	pc += 2;
	unsigned addr = Read(pc-2);
	addr += 0x100*Read(pc-1);
	memptr = addr;
	if (f & SF) {
		Write(--sp, pc_h);
		Write(--sp, pc_l);
		pc = addr;
		t += 13;
	} else t += 6;
}
void OpFE() { // cp nn
	cp8(Read(pc++));
	t += 3;
}
void OpFF() { // rst 38
	Write(--sp, pc_h);
	Write(--sp, pc_l);
	pc = 0x38;
	memptr = 0x38;
	t += 7;
}

#endif//__Z80_OP_NOPREFIX_H__
