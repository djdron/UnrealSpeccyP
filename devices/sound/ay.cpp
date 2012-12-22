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

#include "../../std.h"
#include "../../z80/z80.h"
#include "ay.h"

static struct eOptionSoundChip : public xOptions::eOptionInt
{
	eOptionSoundChip() { Set(SC_AY); }
	enum eType { SC_FIRST, SC_AY = SC_FIRST, SC_YM, SC_LAST };
	virtual const char* Name() const { return "chip"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "ay", "ym", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(SC_LAST, next);
	}
} op_sound_chip;

static struct eOptionAYStereo : public xOptions::eOptionInt
{
	eOptionAYStereo() { Set(AS_ABC); }
	enum eMode { AS_FIRST, AS_ABC = AS_FIRST, AS_ACB, AS_BAC, AS_BCA, AS_CAB, AS_CBA, AS_MONO, AS_LAST };
	virtual const char* Name() const { return "stereo"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "abc", "acb", "bac", "bca", "cab", "cba", "mono", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(AS_LAST, next);
	}
} op_ay_stereo;

const dword SNDR_DEFAULT_AY_RATE = 1774400; // original ZX-Spectrum soundchip clock fq

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

const SNDCHIP_VOLTAB SNDR_VOL_AY_S =
{ { 0x0000,0x0000,0x0340,0x0340,0x04C0,0x04C0,0x06F2,0x06F2,0x0A44,0x0A44,0x0F13,0x0F13,0x1510,0x1510,0x227E,0x227E,
	0x289F,0x289F,0x414E,0x414E,0x5B21,0x5B21,0x7258,0x7258,0x905E,0x905E,0xB550,0xB550,0xD7A0,0xD7A0,0xFFFF,0xFFFF } };

const SNDCHIP_VOLTAB SNDR_VOL_YM_S =
{ { 0x0000,0x0000,0x00EF,0x01D0,0x0290,0x032A,0x03EE,0x04D2,0x0611,0x0782,0x0912,0x0A36,0x0C31,0x0EB6,0x1130,0x13A0,
	0x1751,0x1BF5,0x20E2,0x2594,0x2CA1,0x357F,0x3E45,0x475E,0x5502,0x6620,0x7730,0x8844,0xA1D2,0xC102,0xE0A2,0xFFFF } };

static const SNDCHIP_PANTAB SNDR_PAN_MONO_S = {{100,100, 100,100, 100,100}};
static const SNDCHIP_PANTAB SNDR_PAN_ABC_S =  {{ 100,10,  66,66,   10,100}};
static const SNDCHIP_PANTAB SNDR_PAN_ACB_S =  {{ 100,10,  10,100,  66,66 }};
static const SNDCHIP_PANTAB SNDR_PAN_BAC_S =  {{ 66,66,   100,10,  10,100}};
static const SNDCHIP_PANTAB SNDR_PAN_BCA_S =  {{ 10,100,  100,10,  66,66 }};
static const SNDCHIP_PANTAB SNDR_PAN_CAB_S =  {{ 66,66,   10,100,  100,10}};
static const SNDCHIP_PANTAB SNDR_PAN_CBA_S =  {{ 10,100,  66,66,   100,10}};

const SNDCHIP_VOLTAB* SNDR_VOL_AY = &SNDR_VOL_AY_S;
const SNDCHIP_VOLTAB* SNDR_VOL_YM = &SNDR_VOL_YM_S;
const SNDCHIP_PANTAB* SNDR_PAN_MONO = &SNDR_PAN_MONO_S;
const SNDCHIP_PANTAB* SNDR_PAN_ABC = &SNDR_PAN_ABC_S;
const SNDCHIP_PANTAB* SNDR_PAN_ACB = &SNDR_PAN_ACB_S;
const SNDCHIP_PANTAB* SNDR_PAN_BAC = &SNDR_PAN_BAC_S;
const SNDCHIP_PANTAB* SNDR_PAN_BCA = &SNDR_PAN_BCA_S;
const SNDCHIP_PANTAB* SNDR_PAN_CAB = &SNDR_PAN_CAB_S;
const SNDCHIP_PANTAB* SNDR_PAN_CBA = &SNDR_PAN_CBA_S;

//=============================================================================
//	eAY::eAY
//-----------------------------------------------------------------------------
eAY::eAY()
{
	bitA = bitB = bitC = bitN = 0;
	SetTimings(SNDR_DEFAULT_SYSTICK_RATE, SNDR_DEFAULT_AY_RATE, SNDR_DEFAULT_SAMPLE_RATE);
	SetChip(CHIP_AY);
	SetVolumes(0x7FFF, SNDR_VOL_AY, SNDR_PAN_ABC);
	_Reset();
}
//=============================================================================
//	eAY::IoRead
//-----------------------------------------------------------------------------
bool eAY::IoRead(word port) const
{
	return (port&0xC0FF) == 0xC0FD;
}
//=============================================================================
//	eAY::IoWrite
//-----------------------------------------------------------------------------
bool eAY::IoWrite(word port) const
{
 	if(port&2)
		return false;
 	if((port & 0xC0FF) == 0xC0FD)
		return true;
 	if((port & 0xC000) == 0x8000)
 		return true;
	return false;
}
//=============================================================================
//	eAY::IoRead
//-----------------------------------------------------------------------------
void eAY::IoRead(word port, byte* v, int tact)
{
	*v = Read();
}
//=============================================================================
//	eAY::IoWrite
//-----------------------------------------------------------------------------
void eAY::IoWrite(word port, byte v, int tact)
{
	if((port & 0xC0FF) == 0xC0FD)
	{
		Select(v);
	}
	if((port & 0xC000) == 0x8000)
	{
		Write(tact, v);
	}
}

const dword MULT_C_1 = 14; // fixed point precision for 'system tick -> ay tick'
// b = 1+ln2(max_ay_tick/8) = 1+ln2(max_ay_fq/8 / min_intfq) = 1+ln2(10000000/(10*8)) = 17.9
// assert(b+MULT_C_1 <= 32)

//=============================================================================
//	eAY::FrameStart
//-----------------------------------------------------------------------------
void eAY::FrameStart(dword tacts)
{
	r13_reloaded = 0;
	t = tacts * chip_clock_rate / system_clock_rate;
	eInherited::FrameStart(t);
}
//=============================================================================
//	eAY::FrameEnd
//-----------------------------------------------------------------------------
void eAY::FrameEnd(dword tacts)
{
	//adjusting 't' with whole history will fix accumulation of rounding errors
	qword end_chip_tick = ((passed_clk_ticks + tacts) * chip_clock_rate) / system_clock_rate;
	Flush((dword)(end_chip_tick - passed_chip_ticks));
	eInherited::FrameEnd(t);
	passed_clk_ticks += tacts;
	passed_chip_ticks += t;
}
//=============================================================================
//	eAY::Flush
//-----------------------------------------------------------------------------
void eAY::Flush(dword chiptick)
{
	// todo: noaction at (temp.sndblock || !conf.sound.ay)
	while (t < chiptick)
	{
		t++;
		if(++ta >= fa) ta = 0, bitA ^= -1;
		if(++tb >= fb) tb = 0, bitB ^= -1;
		if(++tc >= fc) tc = 0, bitC ^= -1;
		if(++tn >= fn)
			tn = 0,
			ns = (ns*2+1) ^ (((ns>>16)^(ns>>13)) & 1),
			bitN = 0 - ((ns >> 16) & 1);
		if(++te >= fe)
		{
			te = 0, env += denv;
			if(env & ~31)
			{
				dword mask = (1<<r.env);
				if(mask & ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<15)))
					env = denv = 0;
				else if(mask & ((1<<8)|(1<<12)))
					env &= 31;
				else if(mask & ((1<<10)|(1<<14)))
					denv = -denv, env = env + denv;
				else env = 31, denv = 0; //11,13
			}
		}

		dword en, mix_l, mix_r;

		en = ((ea & env) | va) & ((bitA | bit0) & (bitN | bit3));
		mix_l  = vols[0][en]; mix_r  = vols[1][en];

		en = ((eb & env) | vb) & ((bitB | bit1) & (bitN | bit4));
		mix_l += vols[2][en]; mix_r += vols[3][en];

		en = ((ec & env) | vc) & ((bitC | bit2) & (bitN | bit5));
		mix_l += vols[4][en]; mix_r += vols[5][en];

		if((mix_l ^ eInherited::mix_l) | (mix_r ^ eInherited::mix_r)) // similar check inside update()
			Update(t, mix_l, mix_r);
	}
}
//=============================================================================
//	eAY::Select
//-----------------------------------------------------------------------------
void eAY::Select(byte nreg)
{
	if(chiptype == CHIP_AY) nreg &= 0x0F;
	activereg = nreg;
}
//=============================================================================
//	eAY::Write
//-----------------------------------------------------------------------------
void eAY::Write(dword timestamp, byte val)
{
	if(activereg >= 0x10)
		return;

	if((1 << activereg) & ((1<<1)|(1<<3)|(1<<5)|(1<<13))) val &= 0x0F;
	if((1 << activereg) & ((1<<6)|(1<<8)|(1<<9)|(1<<10))) val &= 0x1F;

	if(activereg != 13 && reg[activereg] == val)
		return;

	reg[activereg] = val;

	if(timestamp)
		Flush((timestamp * mult_const) >> MULT_C_1); // cputick * ( (chip_clock_rate/8) / system_clock_rate );

	switch(activereg)
	{
	case 0:
	case 1:
		fa = SwapWord(r.fA);
		break;
	case 2:
	case 3:
		fb = SwapWord(r.fB);
		break;
	case 4:
	case 5:
		fc = SwapWord(r.fC);
		break;
	case 6:
		fn = val*2;
		break;
	case 7:
		bit0 = 0 - ((val>>0) & 1);
		bit1 = 0 - ((val>>1) & 1);
		bit2 = 0 - ((val>>2) & 1);
		bit3 = 0 - ((val>>3) & 1);
		bit4 = 0 - ((val>>4) & 1);
		bit5 = 0 - ((val>>5) & 1);
		break;
	case 8:
		ea = (val & 0x10)? -1 : 0;
		va = ((val & 0x0F)*2+1) & ~ea;
		break;
	case 9:
		eb = (val & 0x10)? -1 : 0;
		vb = ((val & 0x0F)*2+1) & ~eb;
		break;
	case 10:
		ec = (val & 0x10)? -1 : 0;
		vc = ((val & 0x0F)*2+1) & ~ec;
		break;
	case 11:
	case 12:
		fe = SwapWord(r.envT);
		break;
	case 13:
		r13_reloaded = 1;
		te = 0;
		if(r.env & 4) env = 0, denv = 1; // attack
		else env = 31, denv = -1; // decay
		break;
	}
}
//=============================================================================
//	eAY::Read
//-----------------------------------------------------------------------------
byte eAY::Read()
{
	if(activereg >= 0x10)
		return 0xFF;
	return reg[activereg & 0x0F];
}
//=============================================================================
//	eAY::SetTimings
//-----------------------------------------------------------------------------
void eAY::SetTimings(dword _system_clock_rate, dword _chip_clock_rate, dword _sample_rate)
{
	_chip_clock_rate /= 8;

	system_clock_rate = _system_clock_rate;
	chip_clock_rate = _chip_clock_rate;

	mult_const = (dword)(((qword)chip_clock_rate << MULT_C_1) / system_clock_rate);
	eInherited::SetTimings(_chip_clock_rate, _sample_rate);
	passed_chip_ticks = passed_clk_ticks = 0;
	t = 0; ns = 0xFFFF;

	ApplyRegs();
}
//=============================================================================
//	eAY::SetVolumes
//-----------------------------------------------------------------------------
void eAY::SetVolumes(dword global_vol, const SNDCHIP_VOLTAB *voltab, const SNDCHIP_PANTAB *stereo)
{
	for (int j = 0; j < 6; j++)
		for (int i = 0; i < 32; i++)
			vols[j][i] = (dword)(((qword)global_vol * voltab->v[i] * stereo->raw[j])/(65535*100*3));
}
//=============================================================================
//	eAY::Reset
//-----------------------------------------------------------------------------
void eAY::_Reset(dword timestamp)
{
	for(int i = 0; i < 14; i++)
		reg[i] = 0;
	ApplyRegs(timestamp);
}
//=============================================================================
//	eAY::ApplyRegs
//-----------------------------------------------------------------------------
void eAY::ApplyRegs(dword timestamp)
{
	for(byte r = 0; r < 16; r++)
	{
		Select(r);
		byte p = reg[r];
		/* clr cached values */
		Write(timestamp, p ^ 1);
		Write(timestamp, p);
	}
}
void eAY::OnOption()
{
	bool chip_changed = Option(op_sound_chip);
	bool stereo_changed = Option(op_ay_stereo);
	if(chip_changed || stereo_changed)
	{
		eOptionSoundChip::eType chip = (eOptionSoundChip::eType)(int)op_sound_chip;
		eOptionAYStereo::eMode stereo = (eOptionAYStereo::eMode)(int)op_ay_stereo;
		const SNDCHIP_PANTAB* sndr_pan = SNDR_PAN_MONO;
		switch(stereo)
		{
		case eOptionAYStereo::AS_ABC: sndr_pan = SNDR_PAN_ABC; break;
		case eOptionAYStereo::AS_ACB: sndr_pan = SNDR_PAN_ACB; break;
		case eOptionAYStereo::AS_BAC: sndr_pan = SNDR_PAN_BAC; break;
		case eOptionAYStereo::AS_BCA: sndr_pan = SNDR_PAN_BCA; break;
		case eOptionAYStereo::AS_CAB: sndr_pan = SNDR_PAN_CAB; break;
		case eOptionAYStereo::AS_CBA: sndr_pan = SNDR_PAN_CBA; break;
		case eOptionAYStereo::AS_MONO: sndr_pan = SNDR_PAN_MONO; break;
		case eOptionAYStereo::AS_LAST: break;
		}
		SetChip(chip == eOptionSoundChip::SC_AY ? CHIP_AY : CHIP_YM);
		SetVolumes(0x7FFF, chip == eOptionSoundChip::SC_AY ? SNDR_VOL_AY : SNDR_VOL_YM, sndr_pan);
	}
}
// corresponds enum CHIP_TYPE
const char * const ay_chips[] = { "AY-3-8910", "YM2149F" };
const char* eAY::GetChipName(CHIP_TYPE i) { return ay_chips[i]; }
