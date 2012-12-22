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

#ifndef __AY_H__
#define __AY_H__

#include "../../tools/options.h"

#include "device_sound.h"

#pragma once

namespace xZ80 { class eZ80; }

struct SNDCHIP_VOLTAB;
struct SNDCHIP_PANTAB;
//=============================================================================
//	eAY
//-----------------------------------------------------------------------------
class eAY : public eDeviceSound, public xOptions::eOptionB
{
	typedef eDeviceSound eInherited;
public:
	eAY();
	virtual ~eAY() {}
	virtual bool IoRead(word port) const;
	virtual bool IoWrite(word port) const;
	virtual void IoRead(word port, byte* v, int tact);
	virtual void IoWrite(word port, byte v, int tact);
	void SetRegs(const byte _reg[16]) { memcpy(reg, _reg, sizeof(reg)); ApplyRegs(0); }
	virtual void Reset() { _Reset(); }
	virtual void FrameStart(dword tacts);
	virtual void FrameEnd(dword tacts);

	static eDeviceId Id() { return D_AY; }
	virtual dword IoNeed() const { return ION_WRITE|ION_READ; }
	virtual const char* Name() const { return "ay"; }
protected:
	enum CHIP_TYPE { CHIP_AY, CHIP_YM, CHIP_MAX };
	static const char* GetChipName(CHIP_TYPE i);
	void SetChip(CHIP_TYPE type) { chiptype = type; }
	void SetTimings(dword system_clock_rate, dword chip_clock_rate, dword sample_rate);
	void SetVolumes(dword global_vol, const SNDCHIP_VOLTAB *voltab, const SNDCHIP_PANTAB *stereo);
	void Select(byte nreg);
	void Write(dword timestamp, byte val);
	byte Read();
	void _Reset(dword timestamp = 0); // call with default parameter, when context outside start_frame/end_frame block
	void Flush(dword chiptick);
	void ApplyRegs(dword timestamp = 0);
	virtual void OnOption();
protected:
	dword t, ta, tb, tc, tn, te, env;
	int denv;
	dword bitA, bitB, bitC, bitN, ns;
	dword bit0, bit1, bit2, bit3, bit4, bit5;
	dword ea, eb, ec, va, vb, vc;
	dword fa, fb, fc, fn, fe;
	dword mult_const;

	byte activereg, r13_reloaded;

	dword vols[6][32];
	CHIP_TYPE chiptype;

#pragma pack(push, 1)
	struct AYREGS
	{
		word fA, fB, fC;
		byte noise, mix;
		byte vA, vB, vC;
		word envT;
		byte env;
		byte portA, portB;
	};
#pragma pack(pop)

	union {
		byte reg[16];
		struct AYREGS r;
	};
	dword chip_clock_rate, system_clock_rate;
	qword passed_chip_ticks, passed_clk_ticks;
};

#endif//__AY_H__
