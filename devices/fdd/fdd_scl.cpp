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

//=============================================================================
//	eFdd::AddFile
//-----------------------------------------------------------------------------
bool eFdd::AddFile(const byte* hdr, const byte* data)
{
	Seek(0, 0);
	eUdi::eTrack::eSector* s = Track().GetSector(9);
	if(!s)
		return false;
	int len = hdr[13];
	if(s->DataW(0xe5) < len) //disk full
		return false;
	int pos = s->data[0xe4] * 0x10;
	eUdi::eTrack::eSector* dir = Track().GetSector(1 + pos / 0x100);
	if(!dir)
		return false;
	memcpy(dir->data + (pos & 0xff), hdr, 14);
	dir->DataW((pos & 0xff) + 14, s->DataW(0xe1));
	dir->UpdateCRC();

	pos = s->data[0xe1] + 16 * s->data[0xe2];
	s->data[0xe1] = (pos + len) & 0x0f;
	s->data[0xe2] = (pos + len) >> 4;
	s->data[0xe4]++;
	s->DataW(0xe5, s->DataW(0xe5) - len);
	s->UpdateCRC();

	// goto next track. s8 become invalid
	for(int i = 0; i < len; ++i, ++pos)
	{
		int cyl = pos / 32;
		int side = (pos / 16) & 1;
		Seek(cyl, side);
		if(!Track().WriteSector((pos & 0x0f) + 1, data + i * 0x100))
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
	if(memcmp(data, "SINCLAIR", 8) || data_size < size_t(9 + (0x100 + 14)*buf[8]))
		return false;

	CreateTrd(eUdi::MAX_CYL);
	int size = 0;
	for(int i = 0; i < buf[8]; ++i)
	{
		size += buf[9 + 14 * i + 13];
	}
	Seek(0, 0);
	eUdi::eTrack::eSector* s = Track().GetSector(9);
	s->DataW(0xe5, size); // free sec
	s->UpdateCRC();
	const byte* d = buf + 9 + 14 * buf[8];
	for(int i = 0; i < buf[8]; ++i)
	{
		if(!AddFile(buf + 9 + 14 * i, d))
			return false;
		d += buf[9 + 14 * i + 13] * 0x100;
	}
	return true;
}
//=============================================================================
//	eFdd::ReadScl
//-----------------------------------------------------------------------------
bool eFdd::WriteScl(FILE* file) const
{
	eUdi::eTrack::eSector* s = disk->Track(0, 0).GetSector(9);
	if(!s || !s->data)
		return false;
	byte file_count = s->data[0xe4];
	if(file_count > 128)
		return false;

	byte* buf = new byte[2 * 1024 * 1024];
	memcpy(buf, "SINCLAIR", 8);
	byte* dst = buf + 8;
	*dst++ = file_count;

	struct eFileDesc
	{
		byte len;
		byte sec;
		byte track;
	};
	eFileDesc files[128];
	byte files_found = 0;
	for(int ls = 1; ls < 9; ++ls)
	{
		eUdi::eTrack::eSector* dir = disk->Track(0, 0).GetSector(ls);
		if(!dir || !dir->data)
		{
			SAFE_DELETE_ARRAY(buf);
			return false;
		}
		for(int i = 0; i < 16; ++i)
		{
			const byte* hdr = dir->data + i * 16;
			if(hdr[0] == 0x01) // deleted file
				continue;
			memcpy(dst, hdr, 14);
			dst += 14;
			files[files_found].len = hdr[13];
			files[files_found].sec = hdr[14];
			files[files_found].track = hdr[15];
			++files_found;
			if(files_found == file_count)
				break;
		}
		if(files_found == file_count)
			break;
	}
	for(byte f = 0; f < file_count; ++f)
	{
		const eFileDesc& fd = files[f];
		int pos = fd.sec + 16 * fd.track;
		for(int i = 0; i < fd.len; ++i, ++pos)
		{
			int cyl = disk->Sides() == 2 ? pos / 32 : pos / 16;
			int side = disk->Sides() == 2 ? (pos / 16) & 1 : 0;
			eUdi::eTrack::eSector* s = disk->Track(cyl, side).GetSector((pos & 0x0f) + 1);
			if(!s || !s->data)
			{
				SAFE_DELETE_ARRAY(buf);
				return false;
			}
			memcpy(dst, s->data, 256);
			dst += 256;
		}
	}
	dword sum = 0;
	for(const byte* d = buf; d < dst; ++d)
	{
		sum += *d;
	}
	*dst++ = (sum >> 0) & 0xff;
	*dst++ = (sum >> 8) & 0xff;
	*dst++ = (sum >> 16) & 0xff;
	*dst++ = (sum >> 24) & 0xff;
	size_t size = dst - buf;
	bool ok = fwrite(buf, 1, size, file) == size;
	SAFE_DELETE_ARRAY(buf);
	return ok;
}
