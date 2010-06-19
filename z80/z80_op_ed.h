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

/* ED opcodes */

#ifndef	__Z80_OP_ED_H__
#define	__Z80_OP_ED_H__

#pragma once

void Ope40() { // in b,(c)
	t += 4;
	memptr = bc+1;
	b = IoRead(bc);
	f = log_f[b] | (f & CF);
}
void Ope41() { // out (c),b
	t += 4;
	memptr = bc+1;
	IoWrite(bc, b);
}
void Ope42() { // sbc hl,bc
	memptr = hl+1;
	byte fl = NF;
	fl |= (((hl & 0x0FFF) - (bc & 0x0FFF) - (af & CF)) >> 8) & 0x10; /* HF */
	unsigned tmp = (hl & 0xFFFF) - (bc & 0xFFFF) - (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	int ri = (int)(short)hl - (int)(short)bc - (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope43() { // ld (nnnn),bc
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	Write(adr, c);
	Write(adr+1, b);
	t += 12;
}
void Ope44() { // neg
	f = sbcf[a];
	a = -a;
}
void Ope45() { // retn
	iff1 = iff2;
	unsigned addr = Read(sp++);
	addr += 0x100*Read(sp++);
	pc = addr;
	memptr = addr;
	t += 6;
}
void Ope46() { // im 0
	im = 0;
}
void Ope47() { // ld i,a
	i = a;
	t++;
}
void Ope48() { // in c,(c)
	t += 4;
	memptr = bc+1;
	c = IoRead(bc);
	f = log_f[c] | (f & CF);
}
void Ope49() { // out (c),c
	t += 4;
	memptr = bc+1;
	IoWrite(bc, c);
}
void Ope4A() { // adc hl,bc
	memptr = hl+1;
	byte fl = (((hl & 0x0FFF) + (bc & 0x0FFF) + (af & CF)) >> 8) & 0x10; /* HF */
	unsigned tmp = (hl & 0xFFFF) + (bc & 0xFFFF) + (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	int ri = (int)(short)hl + (int)(short)bc + (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope4B() { // ld bc,(nnnn)
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	c = Read(adr);
	b = Read(adr+1);
	t += 12;
}
void Ope4C() { Ope44(); } // neg
void Ope4D() { // reti
	iff1 = iff2;
	unsigned addr = Read(sp++);
	addr += 0x100*Read(sp++);
	pc = addr;
	memptr = addr;
	t += 6;
}
void Ope4E() { // im 0/1 -> im1
	im = 1;
}
void Ope4F() { // ld r,a
	r_low = a;
	r_hi = a & 0x80;
	t++;
}
void Ope50() { // in d,(c)
	t += 4;
	memptr = bc+1;
	d = IoRead(bc);
	f = log_f[d] | (f & CF);
}
void Ope51() { // out (c),d
	t += 4;
	memptr = bc+1;
	IoWrite(bc, d);
}
void Ope52() { // sbc hl,de
	memptr = hl+1;
	byte fl = NF;
	fl |= (((hl & 0x0FFF) - (de & 0x0FFF) - (af & CF)) >> 8) & 0x10; /* HF */
	unsigned tmp = (hl & 0xFFFF) - (de & 0xFFFF) - (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	int ri = (int)(short)hl - (int)(short)de - (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope53() { // ld (nnnn),de
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	Write(adr, e);
	Write(adr+1, d);
	t += 12;
}
void Ope54() { Ope44(); } // neg
void Ope55() { Ope45(); } // retn
void Ope56() { Ope4E(); } // im 1
void Ope57() { // ld a,i
	a = i;
	f = (log_f[a] | (f & CF)) & ~PV;
	if (iff1 && (t+10 < frame_tacts)) f |= PV;
	t++;
}
void Ope58() { // in e,(c)
	t += 4;
	memptr = bc+1;
	e = IoRead(bc);
	f = log_f[e] | (f & CF);
}
void Ope59() { // out (c),e
	t += 4;
	memptr = bc+1;
	IoWrite(bc, e);
}
void Ope5A() { // adc hl,de
	memptr = hl+1;
	byte fl = (((hl & 0x0FFF) + (de & 0x0FFF) + (af & CF)) >> 8) & 0x10; /* HF */
	unsigned tmp = (hl & 0xFFFF) + (de & 0xFFFF) + (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	int ri = (int)(short)hl + (int)(short)de + (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope5B() { // ld de,(nnnn)
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	e = Read(adr);
	d = Read(adr+1);
	t += 12;
}
void Ope5C() { Ope44(); } // neg
void Ope5D() { Ope4D(); } // reti
void Ope5E() { // im 2
	im = 2;
}
void Ope5F() { // ld a,r
	a = (r_low & 0x7F) | r_hi;
	f = (log_f[a] | (f & CF)) & ~PV;
	if (iff2 && ((t+10 < frame_tacts) || eipos+8==t)) f |= PV;
	t++;
}
void Ope60() { // in h,(c)
	t += 4;
	memptr = bc+1;
	h = IoRead(bc);
	f = log_f[h] | (f & CF);
}
void Ope61() { // out (c),h
	t += 4;
	memptr = bc+1;
	IoWrite(bc, h);
}
void Ope62() { // sbc hl,hl
	memptr = hl+1;
	byte fl = NF;
	fl |= (f & CF) << 4; /* HF - copy from CF */
	unsigned tmp = 0-(af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	// never set PV
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope63() { Op22(); } // ld (nnnn),hl
void Ope64() { Ope44(); } // neg
void Ope65() { Ope45(); } // retn
void Ope66() { Ope46(); } // im 0
void Ope67() { // rrd
	byte tmp = Read(hl);
	memptr = hl+1;
	Write(hl, (a << 4) | (tmp >> 4));
	a = (a & 0xF0) | (tmp & 0x0F);
	f = log_f[a] | (f & CF);
	t += 10;
}
void Ope68() { // in l,(c)
	t += 4;
	memptr = bc+1;
	l = IoRead(bc);
	f = log_f[l] | (f & CF);
}
void Ope69() { // out (c),l
	t += 4;
	memptr = bc+1;
	IoWrite(bc, l);
}
void Ope6A() { // adc hl,hl
	memptr = hl+1;
	byte fl = ((h << 1) & 0x10); /* HF */
	unsigned tmp = (hl & 0xFFFF)*2 + (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	int ri = 2*(int)(short)hl + (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope6B() { Op2A(); } // ld hl,(nnnn)
void Ope6C() { Ope44(); } // neg
void Ope6D() { Ope4D(); } // reti
void Ope6E() { Ope56(); } // im 0/1 -> im 1
void Ope6F() { // rld
	byte tmp = Read(hl);
	memptr = hl+1;
	Write(hl, (a & 0x0F) | (tmp << 4));
	a = (a & 0xF0) | (tmp >> 4);
	f = log_f[a] | (f & CF);
	t += 10;
}
void Ope70() { // in (c)
	t += 4;
	memptr = bc+1;
	f = log_f[IoRead(bc)] | (f & CF);
}
void Ope71() { // out (c),0
	t += 4;
	memptr = bc+1;
	IoWrite(bc, 0);
}
void Ope72() { // sbc hl,sp
	memptr = hl+1;
	byte fl = NF;
	fl |= (((hl & 0x0FFF) - (sp & 0x0FFF) - (af & CF)) >> 8) & 0x10; /* HF */
	unsigned tmp = (hl & 0xFFFF) - (sp & 0xFFFF) - (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(tmp & 0xFFFF)) fl |= ZF;
	int ri = (int)(short)hl - (int)(short)sp - (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope73() { // ld (nnnn),sp
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	Write(adr, sp_l);
	Write(adr+1, sp_h);
	t += 12;
}
void Ope74() { Ope44(); } // neg
void Ope75() { Ope45(); } // retn
void Ope76() { // im 1
	im = 1;
}
void Ope77() { Op00(); } // nop
void Ope78() { // in a,(c)
	t += 4;
	memptr = bc+1;
	a = IoRead(bc);
	f = log_f[a] | (f & CF);
}
void Ope79() { // out (c),a
	t += 4;
	memptr = bc+1;
	IoWrite(bc, a);
}
void Ope7A() { // adc hl,sp
	memptr = hl+1;
	byte fl = (((hl & 0x0FFF) + (sp & 0x0FFF) + (af & CF)) >> 8) & 0x10; /* HF */
	unsigned tmp = (hl & 0xFFFF) + (sp & 0xFFFF) + (af & CF);
	if (tmp & 0x10000) fl |= CF;
	if (!(unsigned short)tmp) fl |= ZF;
	int ri = (int)(short)hl + (int)(short)sp + (int)(af & CF);
	if (ri < -0x8000 || ri >= 0x8000) fl |= PV;
	hl = tmp;
	f = fl | (h & (F3|F5|SF));
	t += 7;
}
void Ope7B() { // ld sp,(nnnn)
	unsigned adr = Read(pc++);
	adr += Read(pc++)*0x100;
	memptr = adr+1;
	sp_l = Read(adr);
	sp_h = Read(adr+1);
	t += 12;
}
void Ope7C() { Ope44(); } // neg
void Ope7D() { Ope4D(); } // reti
void Ope7E() { Ope5E(); } // im 2
void Ope7F() { Op00(); } // nop
void OpeA0() { // ldi
	t += 8;
	byte tempbyte = Read(hl++);
	Write(de++, tempbyte);
	tempbyte += a; tempbyte = (tempbyte & F3) + ((tempbyte << 4) & F5);
	f = (f & ~(NF|HF|PV|F3|F5)) + tempbyte;
	if (--bc & 0xFFFF) f |= PV; //???
}
void OpeA1() { // cpi
	t += 8;
	byte cf = f & CF;
	byte tempbyte = Read(hl++);
	f = cpf8b[a*0x100 + tempbyte] + cf;
	if (--bc & 0xFFFF) f |= PV; //???
	memptr++;
}
void OpeA2() { // ini
	memptr = bc+1;
	t += 8;
	Write(hl++, IoRead(bc));
	dec8(b);
}
void OpeA3() { // outi
	t += 8;
	dec8(b);
	IoWrite(bc, Read(hl++));
	f &= ~CF; if (!l) f |= CF;
	memptr = bc+1;
}
void OpeA8() { // ldd
	t += 8;
	byte tempbyte = Read(hl--);
	Write(de--, tempbyte);
	tempbyte += a; tempbyte = (tempbyte & F3) + ((tempbyte << 4) & F5);
	f = (f & ~(NF|HF|PV|F3|F5)) + tempbyte;
	if (--bc & 0xFFFF) f |= PV; //???
}
void OpeA9() { // cpd
	t += 8;
	byte cf = f & CF;
	byte tempbyte = Read(hl--);
	f = cpf8b[a*0x100 + tempbyte] + cf;
	if (--bc & 0xFFFF) f |= PV; //???
	memptr--;
}
void OpeAA() { // ind
	memptr = bc-1;
	t += 8;
	Write(hl--, IoRead(bc));
	dec8(b);
}
void OpeAB() { // outd
	t += 8;
	dec8(b);
	IoWrite(bc, Read(hl--));
	f &= ~CF; if (l == 0xFF) f |= CF;
	memptr = bc-1;
}
void OpeB0() { // ldir
	t += 8;
	byte tempbyte = Read(hl++);
	Write(de++, tempbyte);
	tempbyte += a; tempbyte = (tempbyte & F3) + ((tempbyte << 4) & F5);
	f = (f & ~(NF|HF|PV|F3|F5)) + tempbyte;
	if (--bc & 0xFFFF) f |= PV, pc -= 2, t += 5, memptr = pc+1; //???
}
void OpeB1() { // cpir
	memptr++;
	t += 8;
	byte cf = f & CF;
	byte tempbyte = Read(hl++);
	f = cpf8b[a*0x100 + tempbyte] + cf;
	if (--bc & 0xFFFF) { //???
		f |= PV;
		if (!(f & ZF)) pc -= 2, t += 5, memptr = pc+1;
	}
}
void OpeB2() { // inir
	t += 8;
	memptr = bc+1;
	Write(hl++, IoRead(bc));
	dec8(b);
	if (b) f |= PV, pc -= 2, t += 5;
	else f &= ~PV;
}
void OpeB3() { // otir
	t += 8;
	dec8(b);
	IoWrite(bc, Read(hl++));
	if (b) f |= PV, pc -= 2, t += 5;
	else f &= ~PV;
	f &= ~CF; if (!l) f |= CF;
	memptr = bc+1;
}
void OpeB8() { // lddr
	t += 8;
	byte tempbyte = Read(hl--);
	Write(de--, tempbyte);
	tempbyte += a; tempbyte = (tempbyte & F3) + ((tempbyte << 4) & F5);
	f = (f & ~(NF|HF|PV|F3|F5)) + tempbyte;
	if (--bc & 0xFFFF) f |= PV, pc -= 2, t += 5; //???
}
void OpeB9() { // cpdr
	memptr--;
	t += 8;
	byte cf = f & CF;
	byte tempbyte = Read(hl--);
	f = cpf8b[a*0x100 + tempbyte] + cf;
	if (--bc & 0xFFFF) { //???
		f |= PV;
		if (!(f & ZF)) pc -= 2, t += 5, memptr = pc+1;
	}
}
void OpeBA() { // indr
	t += 8;
	memptr = bc-1;
	Write(hl--, IoRead(bc));
	dec8(b);
	if (b) f |= PV, pc -= 2, t += 5;
	else f &= ~PV;
}
void OpeBB() { // otdr
	t += 8;
	dec8(b);
	IoWrite(bc, Read(hl--));
	if (b) f |= PV, pc -= 2, t += 5;
	else f &= ~PV;
	f &= ~CF; if (l == 0xFF) f |= CF;
	memptr = bc-1;
}

void OpED()
{
	byte opcode = Fetch();
	(this->*ext_opcodes[opcode])();
}

#endif//__Z80_OP_ED_H__
