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

#ifndef	__Z80_H__
#define	__Z80_H__

#include "z80_op_tables.h"
#include "../platform/endian.h"

#pragma once

class eMemory;
class eRom;
class eUla;
class eDevices;

namespace xZ80
{

#ifdef USE_BIG_ENDIAN
#define DECLARE_REG16(reg, low, high)\
union\
{\
	struct\
	{\
		byte reg##xx;\
		byte reg##x;\
		byte high;\
		byte low;\
	};\
	dword reg;\
};
#else//USE_BIG_ENDIAN
#define DECLARE_REG16(reg, low, high)\
union\
{\
	struct\
	{\
		byte low;\
		byte high;\
		byte reg##x;\
		byte reg##xx;\
	};\
	dword reg;\
};
#endif//USE_BIG_ENDIAN

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

//*****************************************************************************
//	eZ80
//-----------------------------------------------------------------------------
class eZ80
{
public:
	eZ80(eMemory* m, eDevices* d, dword frame_tacts = 0);
	void Reset();
	void Update(int int_len, int* nmi_pending, int* fetches);

	dword FrameTacts() const { return frame_tacts; }
	dword T() const { return t; }

	class eHandlerIo
	{
	public:
		virtual byte Z80_IoRead(word port, int tact) = 0;
	};
	void HandlerIo(eHandlerIo* h) { handler.io = h; }
	eHandlerIo* HandlerIo() const { return handler.io; }

	class eHandlerStep
	{
	public:
		virtual void Z80_Step(eZ80* z80) = 0;
	};
	void HandlerStep(eHandlerStep* h) { handler.step = h; }
	eHandlerStep* HandlerStep() const { return handler.step; }

protected:
	void Int();
	void Nmi();
	void Step();
	void StepF();
	byte Fetch()
	{
		++fetches;
		++r_low;// = (cpu->r & 0x80) + ((cpu->r+1) & 0x7F);
		t += 4;
		return Read(pc++);
	}
	byte IoRead(word port) const;
	void IoWrite(word port, byte v);
	byte Read(word addr) const;
	void Write(word addr, byte v);

	typedef void (eZ80::*CALLFUNC)();
	typedef byte (eZ80::*CALLFUNCI)(byte);

	#include "z80_op.h"
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
	eMemory*	memory;
	eRom*		rom;
	eUla*		ula;
	eDevices*	devices;

	struct eHandler
	{
		eHandler() : io(NULL), step(NULL) {}
		eHandlerIo*	io;
		eHandlerStep* step;
	};
	eHandler handler;

	int		t;
	int		im;
	int		eipos;
	int		haltpos;
	int		frame_tacts;  // t-states per frame
	int		fetches;

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
	struct eAlt
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
	byte reg_unused;
};

}//namespace xZ80

#endif//__Z80_H__
