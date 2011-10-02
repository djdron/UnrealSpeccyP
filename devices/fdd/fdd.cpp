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

#include "fdd.h"

const int trdos_interleave = 1;

#define Min(o, p)	(o < p ? o : p)

//=============================================================================
//	eUdi::eTrack::Marker
//-----------------------------------------------------------------------------
bool eUdi::eTrack::Marker(int pos) const
{
	return (id[pos / 8] & (1 << (pos & 7))) != 0;
}
//=============================================================================
//	eUdi::eTrack::Write
//-----------------------------------------------------------------------------
void eUdi::eTrack::Write(int pos, byte v, bool marker)
{
	if(data)
	{
		data[pos] = v;
		if(marker)
		{
			id[pos / 8] |= 1 << (pos & 7);
		}
		else
		{
			id[pos / 8] &= ~(1 << (pos & 7));
		}
	}
}
//=============================================================================
//	eUdi::eTrack::Update
//-----------------------------------------------------------------------------
void eUdi::eTrack::Update()
{
	byte* src = data;
	int len = data_len - 8;
	sectors_amount = 0;
	int i = 0;
	while(i < len)
	{
		for(; i < len; ++i)
		{
			if(src[i] == 0xa1 && src[i+1] == 0xfe && Marker(i)) //find index data marker
			{
				sectors[sectors_amount].id = src + i + 2;
				sectors[sectors_amount].data = NULL;
				i += 8;
				break;
			}
		}
		int end = Min(len, i + 43); // data marker margin 30-SD, 43-DD
		for(; i < end; ++i)
		{
			if(src[i] == 0xa1 && Marker(i) && !Marker(i + 1)) //find data marker
			{
				if((i < len && src[i+1] == 0xf8) || src[i+1] == 0xfb)
				{
					sectors[sectors_amount].data = src + i + 2;
				}
				break;
			}
		}
		if(sectors_amount++ >= MAX_SEC)
		{
			assert(0); //too many sectors
		}
	}
}

//=============================================================================
//	eUdi::eUdi
//-----------------------------------------------------------------------------
eUdi::eUdi(int _cyls, int _sides) : raw(NULL)
{
	cyls = _cyls; sides = _sides;
	const int max_track_len = 6250;
	int data_len = max_track_len;
	int udi_track_len = data_len + data_len / 8 + ((data_len & 7) ? 1 : 0);
	int size = cyls * sides * udi_track_len;
	raw = new byte[size];
	memset(raw, 0, size);
	for(int i = 0; i < cyls; ++i)
	{
		for(int j = 0; j < sides; ++j)
		{
			eTrack& t = tracks[i][j];
			t.data_len = data_len;
			t.data = raw + udi_track_len * (i * sides + j);
			t.id = t.data + data_len;
		}
	}
}

//=============================================================================
//	eFdd::eFdd
//-----------------------------------------------------------------------------
eFdd::eFdd() : motor(0), cyl(0), side(0), ts_byte(0), write_protect(false), disk(NULL)
{
}
//=============================================================================
//	eFdd::Open
//-----------------------------------------------------------------------------
bool eFdd::Open(const char* type, const void* data, size_t data_size)
{
	if(!strcmp(type, "trd"))
		return ReadTrd(data, data_size);
	if(!strcmp(type, "scl"))
		return ReadScl(data, data_size);
	return true;
}
//=============================================================================
//	eFdd::Seek
//-----------------------------------------------------------------------------
void eFdd::Seek(int _cyl, int _side)
{
	cyl = _cyl;
	side = _side;
	const int Z80FQ = 3500000;
	const int FDD_RPS = 5; // rotation speed
	ts_byte = Z80FQ / (Track().data_len * FDD_RPS);
}
// data misalignment on ARM fighting functions
static inline word SectorDataW(eUdi::eTrack::eSector* s, size_t offset)
{
	return s->data[offset] | word(s->data[offset + 1]) << 8;
}
static inline void SectorDataW(eUdi::eTrack::eSector* s, size_t offset, word d)
{
	s->data[offset] = d & 0xff;
	s->data[offset + 1] = d >> 8;
}
//=============================================================================
//	eFdd::Crc
//-----------------------------------------------------------------------------
word eFdd::Crc(byte* src, int size) const
{
	dword crc = 0xcdb4;
	while(size--)
	{
		crc ^= (*src++) << 8;
		for(int i = 8; i; --i)
		{
			if((crc *= 2) & 0x10000)
			{
				crc ^= 0x1021; // bit representation of x^12+x^5+1
			}
		}
	}
	return crc;
}
//=============================================================================
//	eFdd::Format
//-----------------------------------------------------------------------------
void eFdd::Format()
{
	int id_len = Track().data_len / 8 + ((Track().data_len & 7) ? 1 : 0);
	memset(Track().data, 0, Track().data_len + id_len);

	int pos = 0;
	WriteBlock(pos, 0x4e, 80);		//gap4a
	WriteBlock(pos, 0, 12);			//sync
	WriteBlock(pos, 0xc2, 3, true);	//iam
	Write(pos++, 0xfc);

	const int max_trd_sectors = 16;
	static const byte lv[3][max_trd_sectors] =
	{
		{ 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 },
		{ 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 },
		{ 1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6 }
	};
	Track().sectors_amount = max_trd_sectors;
	for(int i = 0; i < max_trd_sectors; ++i)
	{
		WriteBlock(pos, 0x4e, 40);		//gap1 50 fixme: recalculate gap1 only for non standard formats
		WriteBlock(pos, 0, 12);			//sync
		WriteBlock(pos, 0xa1, 3, true);	//id am
		Write(pos++, 0xfe);
		eUdi::eTrack::eSector& sec = Track().sectors[i];
		sec.id = Track().data + pos;
		Write(pos++, cyl);
		Write(pos++, side);
		Write(pos++, lv[trdos_interleave][i]);
		Write(pos++, 1); //256byte
		word crc = Crc(Track().data + pos - 5, 5);
		Write(pos++, crc >> 8);
		Write(pos++, (byte)crc);

		WriteBlock(pos, 0x4e, 22);		//gap2
		WriteBlock(pos, 0, 12);			//sync
		WriteBlock(pos, 0xa1, 3, true);	//data am
		Write(pos++, 0xfb);
		sec.data = Track().data + pos;
		int len = sec.Len();
		crc = Crc(Track().data + pos - 1, len + 1);
		pos += len;
		Write(pos++, crc >> 8);
		Write(pos++, (byte)crc);
	}
	if(pos > Track().data_len)
	{
		assert(0); //track too long
	}
	WriteBlock(pos, 0x4e, Track().data_len - pos - 1); //gap3
}
//=============================================================================
//	eFdd::FormatTrd
//-----------------------------------------------------------------------------
void eFdd::FormatTrd()
{
	SAFE_DELETE(disk);
	disk = new eUdi(eUdi::MAX_CYL, eUdi::MAX_SIDE);
	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			Seek(i, j);
			Format();
		}
	}
}
//=============================================================================
//	eFdd::WriteSector
//-----------------------------------------------------------------------------
bool eFdd::WriteSector(int cyl, int side, int sec, const byte* data)
{
	eUdi::eTrack::eSector* s = GetSector(cyl, side, sec);
	if(!s || !s->data)
		return false;
	int len = s->Len();
	memcpy(s->data, data, len);
	SectorDataW(s, len, swap_byte_order(Crc(s->data - 1, len + 1)));
	return true;
}
//=============================================================================
//	eFdd::GetSector
//-----------------------------------------------------------------------------
eUdi::eTrack::eSector* eFdd::GetSector(int cyl, int side, int sec)
{
	Seek(cyl, side);
	for(int i = 0; i < Track().sectors_amount; ++i)
	{
		eUdi::eTrack::eSector& s = Track().sectors[i];
		if(s.Sec() == sec && s.Len() == 256)
		{
			return &s;
		}
	}
	return NULL;
}
//=============================================================================
//	eFdd::CreateTrd
//-----------------------------------------------------------------------------
void eFdd::CreateTrd()
{
	FormatTrd();
	eUdi::eTrack::eSector* s = GetSector(0, 0, 9);
	if(!s)
		return;
	s->data[0xe2] = 1;					// first free track
	s->data[0xe3] = 0x16;				// 80T,DS
	SectorDataW(s, 0xe5, 2544);			// free sec
	s->data[0xe7] = 0x10;				// trdos flag
	WriteSector(0, 0, 9, s->data);		// update sector CRC
}
//=============================================================================
//	eFdd::AddFile
//-----------------------------------------------------------------------------
bool eFdd::AddFile(const byte* hdr, const byte* data)
{
	eUdi::eTrack::eSector* s = GetSector(0, 0, 9);
	if(!s)
		return false;
	int len = hdr[13];
	int pos = s->data[0xe4] * 0x10;
	eUdi::eTrack::eSector* dir = GetSector(0, 0, 1 + pos / 0x100);
	if(!dir)
		return false;
	if(SectorDataW(s, 0xe5) < len) //disk full
		return false;
	memcpy(dir->data + (pos & 0xff), hdr, 14);
	SectorDataW(dir, (pos & 0xff) + 14, SectorDataW(s, 0xe1));
	WriteSector(0, 0, 1 + pos / 0x100, dir->data);

	pos = s->data[0xe1] + 16 * s->data[0xe2];
	s->data[0xe1] = (pos + len) & 0x0f, s->data[0xe2] = (pos + len) >> 4;
	s->data[0xe4]++;
	SectorDataW(s, 0xe5, SectorDataW(s, 0xe5) - len);
	WriteSector(0, 0, 9, s->data);

	// goto next track. s8 become invalid
	for(int i = 0; i < len; ++i, ++pos)
	{
		int cyl = pos / 32;
		int side = (pos / 16) & 1;
		if(!WriteSector(cyl, side, (pos & 0x0f) + 1, data + i * 0x100))
			return false;
	}
	return true;
}
//=============================================================================
//	eFdd::ReadScl
//-----------------------------------------------------------------------------
bool eFdd::ReadScl(const void* data, size_t data_size)
{
	if(data_size < 9)
		return false;
	const byte* buf = (const byte*)data;
	if(memcmp(data, "SINCLAIR", 8) || int(data_size) < 9 + (0x100 + 14)*buf[8])
		return false;

	CreateTrd();
	int size = 0;
	for(int i = 0; i < buf[8]; ++i)
	{
		size += buf[9 + 14 * i + 13];
	}
	if(size > 2544)
	{
		eUdi::eTrack::eSector* s = GetSector(0, 0, 9);
		SectorDataW(s, 0xe5, size);			// free sec
		WriteSector(0, 0, 9, s->data);		// update sector CRC
	}
	const byte* d = buf + 9 + 14 * buf[8];
	for(int i = 0; i < buf[8]; ++i)
	{
		if(!AddFile(buf + 9 + 14*i, d))
			return false;
		d += buf[9 + 14*i + 13]*0x100;
	}
	return true;
}
//=============================================================================
//	eFdd::ReadTrd
//-----------------------------------------------------------------------------
bool eFdd::ReadTrd(const void* data, size_t data_size)
{
	CreateTrd();
	enum { TRD_SIZE = 655360 };
	if(data_size > TRD_SIZE)
		data_size = TRD_SIZE;
	for(size_t i = 0; i < data_size; i += 0x100)
	{
		WriteSector(i >> 13, (i >> 12) & 1, ((i >> 8) & 0x0f) + 1, (const byte*)data + i);
	}
	return true;
}
