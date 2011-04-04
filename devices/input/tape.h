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

#ifndef __TAPE_H__
#define __TAPE_H__

#include "../sound/device_sound.h"

#pragma once

class eSpeccy;
namespace xZ80 { class eZ80; class eZ80_FastTape; }

class eTape : public eDeviceSound
{
	typedef eDeviceSound eInherited;
	friend class xZ80::eZ80_FastTape;
public:
	eTape(eSpeccy* s) : speccy(s) {}
	virtual void Init();
	virtual void Reset();
	virtual bool IoRead(word port) const;
	virtual void IoRead(word port, byte* v, int tact);

	bool Open(const char* type, const void* data, size_t data_size);
	void Start();
	void Stop();
	bool Started() const;
	bool Inserted() const;

	static eDeviceId Id() { return D_TAPE; }
	virtual dword IoNeed() const { return ION_READ; }

	byte TapeBit(int tact);
protected:
	bool ParseTAP(const void* data, size_t data_size);
	bool ParseCSW(const void* data, size_t data_size);
	bool ParseTZX(const void* data, size_t data_size);

	dword FindPulse(dword t);
	void FindTapeIndex();
	void FindTapeSizes();
	void StopTape();
	void ResetTape();
	void StartTape();
	void CloseTape();
	void Reserve(dword datasize);
	void MakeBlock(const byte* data, dword size, dword pilot_t,
	      dword s1_t, dword s2_t, dword zero_t, dword one_t,
	      dword pilot_len, dword pause, byte last = 8);
	void Desc(const byte* data, dword size, char* dst);
	void AllocInfocell();
	void NamedCell(const void *nm, dword sz = 0);
	void CreateAppendableBlock();
	void ParseHardware(const byte* ptr);

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

void FastTapeEmul(xZ80::eZ80* z80);

#endif//__TAPE_H__
