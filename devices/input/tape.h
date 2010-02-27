#ifndef __TAPE_H__
#define __TAPE_H__

#include "../sound/device_sound.h"

#pragma once

class eSpeccy;

class eTape : public eDeviceSound
{
	typedef eDeviceSound eInherited;
public:
	eTape(eSpeccy* s) : speccy(s) {}
	virtual void Init();
	virtual void Reset();
	virtual void IoRead(word port, byte* v, int tact);
	bool Open(const char* file);
	void Start();
	void Stop();

	static eDeviceId Id() { return D_TAPE; }

protected:
	bool ParseTAP(byte* buf, size_t buf_size);
	bool ParseCSW(byte* buf, size_t buf_size);
	bool ParseTZX(byte* buf, size_t buf_size);

	dword FindPulse(dword t);
	void FindTapeIndex();
	void FindTapeSizes();
	void StopTape();
	void ResetTape();
	void StartTape();
	void CloseTape();
	void Reserve(dword datasize);
	void MakeBlock(byte* data, dword size, dword pilot_t,
	      dword s1_t, dword s2_t, dword zero_t, dword one_t,
	      dword pilot_len, dword pause, byte last = 8);
	void Desc(byte*data, dword size, char *dst);
	void AllocInfocell();
	void NamedCell(const void *nm, dword sz = 0);
	void CreateAppendableBlock();
	void ParseHardware(byte*ptr);
	byte TapeBit(int tact);

protected:
	eSpeccy* speccy;

	struct eTapeState
	{
		qword edge_change;
		byte* play_pointer; // or NULL if tape stopped
		byte* end_of_tape;  // where to stop tape
		dword index;    // current tape block
		dword tape_bit;
	};
	eTapeState tape;

	struct TAPEINFO
	{
	   char desc[280];
	   dword pos;
	   dword t_size;
	};

	dword tape_pulse[0x100];
	dword max_pulses;
	dword tape_err;

	byte* tape_image;
	dword tape_imagesize;

	TAPEINFO* tapeinfo;
	dword tape_infosize;

	dword appendable;
};

#endif//__TAPE_H__
