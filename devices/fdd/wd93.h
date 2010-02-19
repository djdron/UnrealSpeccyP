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
	virtual void IoRead(word port, byte* v, int tact);
	virtual void IoWrite(word port, byte v, int tact);
	bool Open(const char* image, int fdd);

	static eDeviceId Id() { return D_WD1793; }
protected:
	void	Process(int tact);
	void	FindMarker();
	bool	Ready();
	void	Load();
	void	GetIndex();

	enum eCmdBit
	{
		CB_SEEK_RATE	= 0x03,
		CB_SEEK_VERIFY	= 0x04,
		CB_SEEK_HEADLOAD= 0x08,
		CB_SEEK_TRKUPD	= 0x10,
		CB_SEEK_DIR		= 0x20,

		CB_WRITE_DEL	= 0x01,
		CB_SIDE_CMP_FLAG= 0x02,
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
	enum eRequest { R_DRQ = 0x40, R_INTRQ = 0x80 };
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
	qword	next;
	int		tshift;

	eState	state;
	eState	state2;
	byte	cmd;
	byte	data;
	int		track;
	int		side;				// update this with changing 'system'
	int		sector;
	byte	rqs;
	byte	status;

	int		stepdirection;
	byte	system;				// beta128 system register

	// read/write sector(s) data
	qword	end_waiting_am;
	int		foundid;			// index in trkcache.hdr for next encountered ID and bytes before this ID
	int		rwptr;
	int		rwlen;
	dword	start_crc;			// format track data

	eFdd*	fdd;
	eFdd	fdds[4];
	eSpeccy* speccy;
	eRom*	rom;
};

#endif//__WD1793_H__
