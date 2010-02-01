#ifndef	__TRACK_CACHE_H__
#define	__TRACK_CACHE_H__

#pragma once

struct eSector
{
	byte	c;
	byte	s;
	byte	n;
	byte	l;
	word	crc;
	byte	c1, c2;	// flags: correct CRCs in address and data
	byte*	data;
	byte*	id;
	int		datlen;
	dword	crcd;	// used to load specific CRC from FDI-file
};

enum eSeekMode { JUST_SEEK = 0, LOAD_SECTORS = 1 };

const int MAX_SEC = 256;

class eFdd;

//*****************************************************************************
//	eTrackCache
//-----------------------------------------------------------------------------
class eTrackCache
{
public:
	eTrackCache() { Clear(); }

public:
	void	Clear() { drive = 0; trkd = 0; }
	void	Seek(eFdd* d, int cyl, int side, eSeekMode sm);
	void	Write(int pos, byte v, char index)
	{
		if(!trkd)
			return;
		trkd[pos] = v;
		index ? SetI(pos) : ClrI(pos);
	}
	void	Format();	// before use, call seek(d,c,s,JUST_SEEK), set s and hdr[]
	eSector* GetSector(int sec); // before use, call fill(d,c,s,LOAD_SECTORS)
	int		WriteSector(int sec, byte* data); // call seek(d,c,s,LOAD_SECTORS)

protected:
	void	SetI(int pos) { trki[pos/8] |= 1 << (pos&7); }
	void	ClrI(int pos) { trki[pos/8] &= ~(1 << (pos&7)); }
	byte	TestI(int pos) { return trki[pos/8] & (1 << (pos&7)); }

public:
	int		ts_byte;	// cpu.t per byte

	// generic track data
	int		trklen;
	byte*	trkd;
	int		s;			// no. of sectors
	eSeekMode sm;		// flag: is sectors filled

	// sectors on track
	eSector hdr[MAX_SEC];

protected:
	// cached track position
	eFdd*	drive;
	int		cyl;
	int		side;

	byte*	trki;       // pointer to data inside UDI
};

dword wd93_crc(byte* ptr, unsigned size);

#endif//__TRACK_CACHE_H__
