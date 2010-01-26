#ifndef __AY_H__
#define __AY_H__

#include "device_sound.h"

#pragma once

namespace xZ80 { class eZ80; }

const dword SNDR_DEFAULT_AY_RATE = 1774400; // original ZX-Spectrum soundchip clock fq

struct AYREGS
{
   word fA, fB, fC;
   byte noise, mix;
   byte vA, vB, vC;
   word envT;
   byte env;
   byte portA, portB;
};

struct AYOUT
{
   dword timestamp; // in system ticks
   byte reg_num;
   byte reg_value;
   byte res1, res2; // padding
};

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

class eAY : public eDeviceSound
{
	typedef eDeviceSound eInherited;
public:
	eAY(xZ80::eZ80* cpu);
	virtual ~eAY();

	virtual void IoRead(word port, byte* v);
	virtual void IoWrite(word port, byte v);

	enum CHIP_TYPE { CHIP_AY, CHIP_YM, CHIP_YM2203, CHIP_MAX }; //Dexus
	static const char* GetChipName(CHIP_TYPE i);

	void SetChip(CHIP_TYPE type) { chiptype = type; }
	void SetTimings(dword system_clock_rate, dword chip_clock_rate, dword sample_rate);
	void SetVolumes(dword global_vol, const SNDCHIP_VOLTAB *voltab, const SNDCHIP_PANTAB *stereo);

	void Reset(dword timestamp = 0); // call with default parameter, when context outside start_frame/end_frame block

	// 'render' is a function that converts array of register writes into PCM-buffer
	dword Render(AYOUT *src, dword srclen, dword clk_ticks, bufptr_t dst);

	// set of functions that fills buffer in emulation progress
	virtual void StartFrame(bufptr_t dst);
	void Select(byte nreg);
	void Write(dword timestamp, byte val);
	byte Read();
	virtual dword EndFrame(dword clk_ticks);

 private:
	typedef void YM2203;
	YM2203* chip2203; //registers //Dexus
	//FMSAMPLE FMbuf; //1 sample //Dexus
	enum { FMBUFSIZE = 1 }; //Alone Coder
	short FMbufs[FMBUFSIZE]; //Alone Coder
	word FMbufN; //Alone Coder
	int FMbufOUT; //1 sample for add to AY output //Alone Coder
	word FMbufMUL; //conf.sound.ay/8192*0.7f //Alone Coder
	float nextfmtickfloat,ayticks_per_fmtick; //Alone Coder
	dword nextfmtick; //Alone Coder

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

	void Flush(dword chiptick);
	void ApplyRegs(dword timestamp = 0);

	xZ80::eZ80* cpu;
};

#endif//__AY_H__
