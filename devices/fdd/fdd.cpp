/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2018 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
	for(int i = 0; i < len; ++i)
	{
		if(src[i] == 0xa1 && src[i+1] == 0xfe && Marker(i)) //find index data marker
		{
			assert(sectors_amount < MAX_SEC);
			sectors[sectors_amount].id = src + i + 2;
			sectors[sectors_amount].data = NULL;
			i += 8;
			for(; i < len; ++i)
			{
				if(src[i] == 0xa1 && Marker(i) && !Marker(i + 1)) //find data marker
				{
					if(src[i+1] == 0xf8 || src[i+1] == 0xfb)
					{
						sectors[sectors_amount].data = src + i + 2;
					}
					break;
				}
			}
			++sectors_amount;
		}
	}
}
//=============================================================================
//	eUdi::eTrack::GetSector
//-----------------------------------------------------------------------------
eUdi::eTrack::eSector* eUdi::eTrack::GetSector(int sec)
{
	for(int i = 0; i < sectors_amount; ++i)
	{
		eUdi::eTrack::eSector& s = sectors[i];
		if(s.id && s.Sec() == sec && s.Len() == 256)
		{
			return &s;
		}
	}
	return NULL;
}
//=============================================================================
//	eUdi::eTrack::WriteSector
//-----------------------------------------------------------------------------
bool eUdi::eTrack::WriteSector(int sec, const byte* data)
{
	eUdi::eTrack::eSector* s = GetSector(sec);
	if(!s || !s->data)
		return false;
	int len = s->Len();
	memcpy(s->data, data, len);
	s->UpdateCRC();
	return true;
}
//=============================================================================
//	eUdi::eTrack::eSector::UpdateCRC
//-----------------------------------------------------------------------------
void eUdi::eTrack::eSector::UpdateCRC()
{
	int len = Len();
	DataW(len, swap_byte_order(eFdd::Crc(data - 1, len + 1)));
}

//=============================================================================
//	eUdi::eUdi
//-----------------------------------------------------------------------------
eUdi::eUdi(int _cyls, int _sides) : raw(NULL), changed(false)
{
	cyls = _cyls; sides = _sides;
	const int max_track_len = 6400;
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
	Motor(0);
	bool ok = false;
	if(!strcmp(type, "trd"))
		ok = ReadTrd(data, data_size);
	else if(!strcmp(type, "scl"))
		ok = ReadScl(data, data_size);
	else if(!strcmp(type, "fdi"))
		ok = ReadFdi(data, data_size);
	else if(!strcmp(type, "udi"))
		ok = ReadUdi(data, data_size);
	else if(!strcmp(type, "td0"))
		ok = ReadTd0(data, data_size);
	SAFE_CALL(disk)->Changed(false);
	return ok;
}
//=============================================================================
//	eFdd::Store
//-----------------------------------------------------------------------------
bool eFdd::Store(const char* type, FILE* file) const
{
	if(!DiskPresent())
		return false;
	bool ok = false;
	if(!strcmp(type, "trd"))
		ok = WriteTrd(file);
	else if(!strcmp(type, "scl"))
		ok = WriteScl(file);
	else if(!strcmp(type, "fdi"))
		ok = WriteFdi(file);
	else if(!strcmp(type, "udi"))
		ok = WriteUdi(file);
	else if(!strcmp(type, "td0"))
		ok = WriteTd0(file);
	if(ok)
		disk->Changed(false);
	return ok;
}
//=============================================================================
//	eFdd::Bootable
//-----------------------------------------------------------------------------
static const char* boot_sign = "boot    B";
bool eFdd::Bootable() const
{
	for(int i = 1; i < 9; ++i)
	{
		const eUdi::eTrack::eSector* s = disk->Track(0, 0).GetSector(i);
		if(!s || !s->data)
			return true; // unknown format may be bootable
		for(const byte* ptr = s->data; ptr < s->data + 256; ptr += 16)
		{
			if(memcmp(ptr, boot_sign, 9) == 0)
				return true;
		}
	}
	return false;
}
//=============================================================================
//	eFdd::DiskChanged
//-----------------------------------------------------------------------------
bool eFdd::DiskChanged() const
{
	return disk && disk->Changed();
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
//=============================================================================
//	eFdd::Crc
//-----------------------------------------------------------------------------
word eFdd::Crc(byte* src, int size)
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
