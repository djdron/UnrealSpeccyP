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

/* DDCB/FDCB opcodes */
/* note: cpu.t and destination updated in step(), here process 'v' */

#ifndef	__Z80_OP_DDCB_H__
#define	__Z80_OP_DDCB_H__

#pragma once

byte Oplx00(byte v) { // rlc (ix+nn)
	f = rlcf[v]; return rol[v];
}
byte Oplx08(byte v) { // rrc (ix+nn)
	f = rrcf[v]; return ror[v];
}
byte Oplx10(byte v) { // rl (ix+nn)
	if (f & CF) {
		f = rl1[v]; return (v << 1) + 1;
	} else {
		f = rl0[v]; return (v << 1);
	}
}
byte Oplx18(byte v) { // rr (ix+nn)
	if (f & CF) {
		f = rr1[v]; return (v >> 1) + 0x80;
	} else {
		f = rr0[v]; return (v >> 1);
	}
}
byte Oplx20(byte v) { // sla (ix+nn)
	f = rl0[v]; return (v << 1);
}
byte Oplx28(byte v) { // sra (ix+nn)
	f = sraf[v]; return (v >> 1) + (v & 0x80);
}
byte Oplx30(byte v) { // sli (ix+nn)
	f = rl1[v]; return (v << 1) + 1;
}
byte Oplx38(byte v) { // srl (ix+nn)
	f = rr0[v]; return (v >> 1);
}
byte Oplx40(byte v) { // bit 0,(ix+nn)
	bitmem(v, 0); return v;
}
byte Oplx48(byte v) { // bit 1,(ix+nn)
	bitmem(v, 1); return v;
}
byte Oplx50(byte v) { // bit 2,(ix+nn)
	bitmem(v, 2); return v;
}
byte Oplx58(byte v) { // bit 3,(ix+nn)
	bitmem(v, 3); return v;
}
byte Oplx60(byte v) { // bit 4,(ix+nn)
	bitmem(v, 4); return v;
}
byte Oplx68(byte v) { // bit 5,(ix+nn)
	bitmem(v, 5); return v;
}
byte Oplx70(byte v) { // bit 6,(ix+nn)
	bitmem(v, 6); return v;
}
byte Oplx78(byte v) { // bit 7,(ix+nn)
	bitmem(v, 7); return v;
}
byte Oplx80(byte v) { // res 0,(ix+nn)
	return resbyte(v, 0);
}
byte Oplx88(byte v) { // res 1,(ix+nn)
	return resbyte(v, 1);
}
byte Oplx90(byte v) { // res 2,(ix+nn)
	return resbyte(v, 2);
}
byte Oplx98(byte v) { // res 3,(ix+nn)
	return resbyte(v, 3);
}
byte OplxA0(byte v) { // res 4,(ix+nn)
	return resbyte(v, 4);
}
byte OplxA8(byte v) { // res 5,(ix+nn)
	return resbyte(v, 5);
}
byte OplxB0(byte v) { // res 6,(ix+nn)
	return resbyte(v, 6);
}
byte OplxB8(byte v) { // res 7,(ix+nn)
	return resbyte(v, 7);
}
byte OplxC0(byte v) { // set 0,(ix+nn)
	return setbyte(v, 0);
}
byte OplxC8(byte v) { // set 1,(ix+nn)
	return setbyte(v, 1);
}
byte OplxD0(byte v) { // set 2,(ix+nn)
	return setbyte(v, 2);
}
byte OplxD8(byte v) { // set 3,(ix+nn)
	return setbyte(v, 3);
}
byte OplxE0(byte v) { // set 4,(ix+nn)
	return setbyte(v, 4);
}
byte OplxE8(byte v) { // set 5,(ix+nn)
	return setbyte(v, 5);
}
byte OplxF0(byte v) { // set 6,(ix+nn)
	return setbyte(v, 6);
}
byte OplxF8(byte v) { // set 7,(ix+nn)
	return setbyte(v, 7);
}

inline void DDFD(byte opcode)
{
	byte op1; // last DD/FD prefix
	do
	{
		op1 = opcode;
		opcode = Fetch();
	} while((opcode | 0x20) == 0xFD); // opcode == DD/FD

	if(opcode == 0xCB)
	{
		dword ptr; // pointer to DDCB operand
		ptr = ((op1 == 0xDD) ? ix : iy) + (signed char)Read(pc++);
		memptr = ptr;
		// DDCBnnXX,FDCBnnXX increment R by 2, not 3!
		opcode = Fetch();
		--r_low;
		--fetches;
		byte v = (this->*logic_ix_opcodes[opcode])(Read(ptr));
		if((opcode & 0xC0) == 0x40) { t += 8; return; } // bit n,rm

		// select destination register for shift/res/set
		(this->*reg_offset[opcode & 7]) = v; // ???
		Write(ptr, v);
		t += 11;
		return;
	}
	if(opcode == 0xED)
	{
		OpED();
		return;
	}
	// one prefix: DD/FD
	op1 == 0xDD ? (this->*ix_opcodes[opcode])() : (this->*iy_opcodes[opcode])();
}
inline void OpDD()
{
	DDFD(0xDD);
}
inline void OpFD()
{
	DDFD(0xFD);
}

#endif//__Z80_OP_DDCB_H__
