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

#ifndef	__WD1793_H__
#define	__WD1793_H__

#include "../device.h"
#include "fdd.h"

#pragma once

class eSpeccy;
class eRom;

//*****************************************************************************
//	WD1793
//-----------------------------------------------------------------------------
class eWD1793 : public eDevice
{
public:
	eWD1793(eSpeccy* _speccy, eRom* _rom);
	virtual void Init();
	virtual bool IoRead(word port) const;
	virtual bool IoWrite(word port) const;
	virtual void IoRead(word port, byte* v, int tact);
	virtual void IoWrite(word port, byte v, int tact);
	bool Open(const char* type, int drive, const void* data, size_t data_size);

	static eDeviceId Id() { return D_WD1793; }
	virtual dword IoNeed() const { return ION_WRITE|ION_READ; }
protected:
	void	Process(int tact);
	void	ReadFirstByte();
	void	FindMarker();
	bool	Ready();
	void	Load();
	void	GetIndex();
	word	Crc(byte* src, int size) const;
	word	Crc(byte v, word prev) const;

	enum eCmdBit
	{
		CB_SEEK_RATE	= 0x03,
		CB_SEEK_VERIFY	= 0x04,
		CB_SEEK_HEADLOAD= 0x08,
		CB_SEEK_TRKUPD	= 0x10,
		CB_SEEK_DIR		= 0x20,

		CB_WRITE_DEL	= 0x01,
		CB_SIDE_CMP		= 0x02,
		CB_DELAY		= 0x04,
		CB_SIDE			= 0x08,
		CB_SIDE_SHIFT	= 3,
		CB_MULTIPLE		= 0x10
	};
	enum eState
	{
		S_IDLE, S_WAIT, S_DELAY_BEFORE_CMD, S_CMD_RW, S_FOUND_NEXT_ID,
		S_READ, S_WRSEC, S_WRITE, S_WRTRACK, S_WR_TRACK_DATA, S_TYPE1_CMD,
		S_STEP, S_SEEKSTART, S_SEEK, S_VERIFY, S_RESET
	};
	enum eRequest { R_NONE, R_DRQ = 0x40, R_INTRQ = 0x80 };
	enum eStatus
	{
		ST_BUSY		= 0x01,
		ST_INDEX	= 0x02,
		ST_DRQ		= 0x02,
		ST_TRK00	= 0x04,
		ST_LOST		= 0x04,
		ST_CRCERR	= 0x08,
		ST_NOTFOUND	= 0x10,
		ST_SEEKERR	= 0x10,
		ST_RECORDT	= 0x20,
		ST_HEADL	= 0x20,
		ST_WRFAULT	= 0x20,
		ST_WRITEP	= 0x40,
		ST_NOTRDY	= 0x80
	};
protected:
	eSpeccy* speccy;
	eRom*	rom;

	qword	next;
	int		tshift;

	eState	state;
	eState	state_next;
	byte	cmd;
	byte	data;
	int		track;
	int		side;				// update this with changing 'system'
	int		sector;
	int		direction;

	byte	rqs;
	byte	status;
	byte	system;				// beta128 system register

	// read/write sector(s) data
	qword	end_waiting_am;
	eUdi::eTrack::eSector* found_sec;
	int		rwptr;
	int		rwlen;
	word	crc;
	int		start_crc;

	enum { FDD_COUNT = 4 };
	eFdd*	fdd;
	eFdd	fdds[FDD_COUNT];
};

#endif//__WD1793_H__
