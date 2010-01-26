#ifndef __DEVICE_SOUND_H__
#define __DEVICE_SOUND_H__

#include "../device.h"

#pragma once

union SNDSAMPLE
{
   dword sample; // left/right channels in low/high WORDs
   struct { word left, right; } ch; // or left/right separately
};

struct SNDOUT
{
   dword timestamp; // in 'system clock' ticks
   SNDSAMPLE newvalue;
};

typedef SNDSAMPLE* bufptr_t;

const dword SNDR_DEFAULT_SYSTICK_RATE = 3500000; // ZX-Spectrum Z80 clock
const dword SNDR_DEFAULT_SAMPLE_RATE = 44100;

class eDeviceSound : public eDevice
{
public:
	eDeviceSound();
	void SetTimings(dword clock_rate, dword sample_rate);

	// 'render' is a function that converts array of DAC inputs into PCM-buffer
	dword Render(SNDOUT *src, dword srclen, dword clk_ticks, bufptr_t dst);

	// set of functions that fills buffer in emulation progress
	virtual void StartFrame(bufptr_t dst);
	void Update(dword timestamp, dword l, dword r);
	virtual dword EndFrame(dword clk_ticks);

	// new start position as previous end position
	// (continue to fill buffer)
	void StartFrame() { StartFrame(dstpos); }

	enum { BUFFER_LEN = 16384 };

	void* AudioData();
	dword AudioDataReady();
	void AudioDataUse(dword size);

protected:
	dword mix_l, mix_r;
	bufptr_t dstpos, dst_start;
	dword clock_rate, sample_rate; //Alone Coder

	SNDSAMPLE buffer[BUFFER_LEN];

private:
	dword tick, base_tick;
	dword s1_l, s1_r;
	dword s2_l, s2_r;
	dword firstsmp; //Alone Coder
	int oldleft,useleft,olduseleft,oldfrmleft; //Alone Coder
	int oldright,useright,olduseright,oldfrmright; //Alone Coder

	//   dword clock_rate, sample_rate;
	qword passed_clk_ticks, passed_snd_ticks;
	dword mult_const;

	void Flush(dword endtick);
};

#endif//__DEVICE_SOUND_H__
