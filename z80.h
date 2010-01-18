#ifndef	__Z80_H__
#define	__Z80_H__

#include "z80_op_tables.h"

#pragma once

namespace xZ80
{

#define DECLARE_REG16(reg, low, high)	\
union	\
{	\
	struct	\
	{		\
		byte low;	\
		byte high;	\
	};	\
	dword reg;	\
};

enum eFlags
{
	CF = 0x01,
	NF = 0x02,
	PV = 0x04,
	F3 = 0x08,
	HF = 0x10,
	F5 = 0x20,
	ZF = 0x40,
	SF = 0x80
};

extern byte even_m1; // m1_wait = 0xC0 - on wait machines, 0 - no_wait on pentagon
extern bool unstable_databus;

//*****************************************************************************
//	eZ80
//-----------------------------------------------------------------------------
class eZ80
{
public:
	eZ80(dword frame_tacts = 0);
	void Reset();
	void Update(dword int_len, int* nmi_pending);

protected:
	void Int();
	void Nmi();
	void Step();

	byte Fetch(); //m1_cycle data fetch
	byte In(word port) const;
	void Out(word port, byte v);
	byte Read(word addr) const;
	void Write(word addr, byte v);

	void inc8(byte& x);
	void dec8(byte& x);
	void add8(byte src);
	void adc8(byte src);
	void sub8(byte src);
	void sbc8(byte src);
	void and8(byte src);
	void or8(byte src);
	void xor8(byte src);
	void cp8(byte src);
	void bit(byte src, byte bit);
	void bitmem(byte src, byte bit);

	void res(byte& src, byte bit) const;
	byte resbyte(byte src, byte bit) const;
	void set(byte& src, byte bit) const;
	byte setbyte(byte src, byte bit) const;

	typedef void (eZ80::*CALLFUNC)();
	typedef byte (eZ80::*CALLFUNCI)(byte);

	#include "z80_op_noprefix.h"
	#include "z80_op_cb.h"
	#include "z80_op_dd.h"
	#include "z80_op_ed.h"
	#include "z80_op_fd.h"
	#include "z80_op_ddcb.h"

	void InitOpNoPrefix();
	void InitOpCB();
	void InitOpDD();
	void InitOpED();
	void InitOpFD();
	void InitOpDDCB();

protected:
	dword	t;
	byte	im;
	dword	eipos;
	dword	haltpos;
	dword	frame_tacts;  // t-states per frame
	unsigned short last_branch;	//? dbg

	DECLARE_REG16(pc, pc_l, pc_h)
	DECLARE_REG16(sp, sp_l, sp_h)
	DECLARE_REG16(ir, r_low, i)
	union
	{
		struct
		{
			byte r_hi;
			byte iff1;
			byte iff2;
			byte halted;
		};
		dword int_flags;
	};
	DECLARE_REG16(memptr, mem_l, mem_h)	// undocumented register
	DECLARE_REG16(ix, xl, xh)
	DECLARE_REG16(iy, yl, yh)

	DECLARE_REG16(bc, c, b)
	DECLARE_REG16(de, e, d)
	DECLARE_REG16(hl, l, h)
	DECLARE_REG16(af, f, a)
	struct
	{
		DECLARE_REG16(bc, c, b)
		DECLARE_REG16(de, e, d)
		DECLARE_REG16(hl, l, h)
		DECLARE_REG16(af, f, a)
	} alt;

	CALLFUNC normal_opcodes[0x100];
	CALLFUNC logic_opcodes[0x100];
	CALLFUNC ix_opcodes[0x100];
	CALLFUNC iy_opcodes[0x100];
	CALLFUNC ext_opcodes[0x100];
	CALLFUNCI logic_ix_opcodes[0x100];

	typedef byte (eZ80::*REGP);
	REGP reg_offset[8];
};

}//namespace xZ80

#endif//__Z80_H__
