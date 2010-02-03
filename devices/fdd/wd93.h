#ifndef	__WD93_H__
#define	__WD93_H__

#include "../device.h"
#include "fdd.h"
#include "track_cache.h"

#pragma once

class eSpeccy;
class eRom;

//*****************************************************************************
//	WD1793
//-----------------------------------------------------------------------------
class eWD1793 : public eDevice
{
public:
	eWD1793(eSpeccy* _speccy, eRom* _rom) : speccy(_speccy), rom(_rom) { seldrive = &fdd[0]; }
	virtual void IoRead(word port, byte* v, int tact);
	virtual void IoWrite(word port, byte v, int tact);

	bool OpenImage(int fdd_index, const char* name);
	static eDeviceId Id() { return D_WD1793; }
protected:
	void	Process(int tact);
	void	FindMarker();
	bool	Ready();
	void	Load();
	void	GetIndex();

	enum CMDBITS
	{
		CMD_SEEK_RATE     = 0x03,
		CMD_SEEK_VERIFY   = 0x04,
		CMD_SEEK_HEADLOAD = 0x08,
		CMD_SEEK_TRKUPD   = 0x10,
		CMD_SEEK_DIR      = 0x20,

		CMD_WRITE_DEL     = 0x01,
		CMD_SIDE_CMP_FLAG = 0x02,
		CMD_DELAY         = 0x04,
		CMD_SIDE          = 0x08,
		CMD_SIDE_SHIFT    = 3,
		CMD_MULTIPLE      = 0x10
	};

	enum WDSTATE
	{
		S_IDLE = 0,
		S_WAIT,

		S_DELAY_BEFORE_CMD,
		S_CMD_RW,
		S_FOUND_NEXT_ID,
		S_READ,
		S_WRSEC,
		S_WRITE,
		S_WRTRACK,
		S_WR_TRACK_DATA,

		S_TYPE1_CMD,
		S_STEP,
		S_SEEKSTART,
		S_SEEK,
		S_VERIFY,

		S_RESET
	};

	enum BETA_STATUS
	{
		DRQ   = 0x40,
		INTRQ = 0x80
	};

	enum WD_STATUS
	{
		WDS_BUSY      = 0x01,
		WDS_INDEX     = 0x02,
		WDS_DRQ       = 0x02,
		WDS_TRK00     = 0x04,
		WDS_LOST      = 0x04,
		WDS_CRCERR    = 0x08,
		WDS_NOTFOUND  = 0x10,
		WDS_SEEKERR   = 0x10,
		WDS_RECORDT   = 0x20,
		WDS_HEADL     = 0x20,
		WDS_WRFAULT   = 0x20,
		WDS_WRITEP    = 0x40,
		WDS_NOTRDY    = 0x80
	};
protected:
	qword	next, time;
	int		tshift;

	byte	state, state2, cmd;
	byte	data, track, sector;
	byte	rqs, status;

	int		drive, side;		// update this with changing 'system'

	char	stepdirection;
	byte	system;				// beta128 system register

	// read/write sector(s) data
	qword	end_waiting_am;
	int		foundid;			// index in trkcache.hdr for next encountered ID and bytes before this ID
	int		rwptr, rwlen;

	// format track data
	dword	start_crc;

	eTrackCache trkcache;
	eFdd*	seldrive;
	eFdd	fdd[4];

	eSpeccy* speccy;
	eRom*	rom;
};

#endif//__WD93_H__
