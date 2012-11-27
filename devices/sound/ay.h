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

#include "device_sound.h"

#pragma once

namespace xZ80 { class eZ80; }

const dword SNDR_DEFAULT_AY_RATE = 1774400; // original ZX-Spectrum soundchip clock fq

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

// output volumes (#0000-#FFFF) for given envelope state or R8-R10 value
// AY chip has only 16 different volume values, so v[0]=v[1], v[2]=v[3], ...
struct SNDCHIP_VOLTAB
{
	dword v[32];
};

// generator's channel panning, % (0-100)
struct SNDCHIP_PANTAB
{
	dword raw[6];
	// structured as 'struct { dword left, right; } chan[3]';
};

// used as parameters to SNDCHIP::set_volumes(),
// if application don't want to override defaults
extern const SNDCHIP_VOLTAB* SNDR_VOL_AY;
extern const SNDCHIP_VOLTAB* SNDR_VOL_YM;

extern const SNDCHIP_PANTAB* SNDR_PAN_MONO;
extern const SNDCHIP_PANTAB* SNDR_PAN_ABC;
extern const SNDCHIP_PANTAB* SNDR_PAN_ACB;
extern const SNDCHIP_PANTAB* SNDR_PAN_BAC;
extern const SNDCHIP_PANTAB* SNDR_PAN_BCA;
extern const SNDCHIP_PANTAB* SNDR_PAN_CAB;
extern const SNDCHIP_PANTAB* SNDR_PAN_CBA;

//=============================================================================
//	eAY
//-----------------------------------------------------------------------------
class eAY : public eDeviceSound
{
	typedef eDeviceSound eInherited;
public:
	eAY();
	virtual ~eAY() {}

	virtual bool IoRead(word port) const;
	virtual bool IoWrite(word port) const;
	virtual void IoRead(word port, byte* v, int tact);
	virtual void IoWrite(word port, byte v, int tact);

	enum CHIP_TYPE { CHIP_AY, CHIP_YM, CHIP_MAX };
	static const char* GetChipName(CHIP_TYPE i);

	void SetChip(CHIP_TYPE type) { chiptype = type; }
	void SetTimings(dword system_clock_rate, dword chip_clock_rate, dword sample_rate);
	void SetVolumes(dword global_vol, const SNDCHIP_VOLTAB *voltab, const SNDCHIP_PANTAB *stereo);

	virtual void Reset() { _Reset(); }
	virtual void FrameStart(dword tacts);
	virtual void FrameEnd(dword tacts);

	static eDeviceId Id() { return D_AY; }
	virtual dword IoNeed() const { return ION_WRITE|ION_READ; }
protected:
	void Select(byte nreg);
	void Write(dword timestamp, byte val);
	byte Read();

private:
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

	union {
		byte reg[16];
		struct AYREGS r;
	};

	dword chip_clock_rate, system_clock_rate;
	qword passed_chip_ticks, passed_clk_ticks;

	void _Reset(dword timestamp = 0); // call with default parameter, when context outside start_frame/end_frame block
	void Flush(dword chiptick);
	void ApplyRegs(dword timestamp = 0);
};

#endif//__AY_H__
