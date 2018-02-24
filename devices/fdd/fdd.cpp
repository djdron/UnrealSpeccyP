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
			if(sectors_amount++ >= MAX_SEC)
			{
				assert(0); //too many sectors
			}
		}
	}
}

//=============================================================================
//	eUdi::eUdi
//-----------------------------------------------------------------------------
eUdi::eUdi(int _cyls, int _sides) : raw(NULL)
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
	if(!strcmp(type, "trd"))
		return ReadTrd(data, data_size);
	if(!strcmp(type, "scl"))
		return ReadScl(data, data_size);
	if(!strcmp(type, "fdi"))
		return ReadFdi(data, data_size);
	if(!strcmp(type, "udi"))
		return ReadUdi(data, data_size);
	if(!strcmp(type, "td0"))
		return ReadTd0(data, data_size);
	return false;
}
//=============================================================================
//	eFdd::Store
//-----------------------------------------------------------------------------
bool eFdd::Store(const char* type, FILE* file) const
{
	if(!strcmp(type, "trd"))
		return WriteTrd(file);
	if(!strcmp(type, "scl"))
		return WriteScl(file);
	if(!strcmp(type, "fdi"))
		return WriteFdi(file);
	if(!strcmp(type, "udi"))
		return WriteUdi(file);
	if(!strcmp(type, "td0"))
		return WriteTd0(file);
	return false;
}
//=============================================================================
//	eFdd::BootExist
//-----------------------------------------------------------------------------
static const char* boot_sign = "boot    B";
bool eFdd::BootExist()
{
	for(int i = 0; i < 9; ++i)
	{
		const eUdi::eTrack::eSector* s = GetSector(0, 0, i);
		if(!s)
			continue;
		const byte* ptr = s->data;
		const char* b = boot_sign;
		for(; (ptr = (const byte*)memchr(ptr, *b, s->Len() - (ptr - s->data))) != NULL; ++b)
		{
			if(!*b)
				return true;
		}
	}
	return false;
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
//	eFdd::WriteSector
//-----------------------------------------------------------------------------
bool eFdd::WriteSector(int cyl, int side, int sec, const byte* data)
{
	eUdi::eTrack::eSector* s = GetSector(cyl, side, sec);
	if(!s || !s->data)
		return false;
	int len = s->Len();
	memcpy(s->data, data, len);
	UpdateCRC(s);
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
		eUdi::eTrack::eSector& s = Sector(i);
		if(s.id && s.Sec() == sec && s.Len() == 256)
		{
			return &s;
		}
	}
	return NULL;
}
//=============================================================================
//	eFdd::UpdateCRC
//-----------------------------------------------------------------------------
void eFdd::UpdateCRC(eUdi::eTrack::eSector* s) const
{
	int len = s->Len();
	s->DataW(len, swap_byte_order(Crc(s->data - 1, len + 1)));
}
