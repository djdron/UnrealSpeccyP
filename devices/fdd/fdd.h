#ifndef	__FDD_H__
#define	__FDD_H__

#include "track_cache.h"

#pragma once

const int MAX_CYLS = 86; // don't load images with so many tracks

//*****************************************************************************
//	eFdd
//-----------------------------------------------------------------------------
class eFdd
{
public:
	~eFdd() { Free(); }
	void Init();

protected:
	void	Free();

	void	FormatTrd();
	void	EmptyDisk();
	void	NewDisk(int cyls, int sides);
	bool	AddFile(byte* hdr, byte* data);

	bool	ReadScl(byte* snbuf);
	bool	ReadTrd(byte* snbuf);

public:
	// drive data
	qword	motor;	// 0 - not spinning, >0 - time when it'll stop
	byte	track;	// head position
	byte*	rawdata;
	byte	optype;	// bits: 0-not modified, 1-write sector, 2-format track

	// disk data
	int		cyls;
	int		trklen[MAX_CYLS][2];
	byte*	trkd[MAX_CYLS][2];
	byte*	trki[MAX_CYLS][2];

protected:
	// disk data
	int		rawsize;
	int		sides;
	byte	snaptype;

	eTrackCache t;	// used in read/write image
	char	name[0x200];
	char	dsc[0x200];

	enum { TRD_SIZE = 655360 };
};

#endif//__FDD_H__
