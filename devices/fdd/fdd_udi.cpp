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
//	eFdd::ReadUdi
//-----------------------------------------------------------------------------
bool eFdd::ReadUdi(const void* data, size_t data_size)
{
	const byte* buf = (const byte*)data;
	SAFE_DELETE(disk);
	disk = new eUdi(buf[9] + 1, buf[10] + 1);

	const byte* ptr = buf + 0x10;
	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			Seek(i, j);
			eUdi::eTrack& t = Track();
			byte* dst = t.data;
			word data_len = Word(ptr + 1);
			t.data_len = data_len;
			t.id = t.data + data_len;
			data_len += data_len / 8 + ((data_len & 7) ? 1 : 0);
			memcpy(dst, ptr + 3, data_len);
			ptr += 3 + data_len;
			t.Update();
		}
	}
	return true;
}

//=============================================================================
//	udi_buggy_crc
//-----------------------------------------------------------------------------
static int udi_buggy_crc(int crc, const byte* buf, size_t len)
{
	while(len--)
	{
		crc ^= -1 ^ *buf++;
		for(int k = 8; k--;)
		{
			int temp = -(crc & 1);
			crc >>= 1;
			crc ^= 0xEDB88320 & temp;
		}
		crc ^= -1;
	}
	return crc;
}

//=============================================================================
//	eFdd::WriteUdi
//-----------------------------------------------------------------------------
bool eFdd::WriteUdi(FILE* file) const
{
	byte* buf = new byte[2*1024*1024];
	memset(buf, 0, 16);
	buf[0] = 'U'; buf[1] = 'D'; buf[2] = 'I'; buf[3] = '!';
	buf[9] = disk->Cyls() - 1;
	buf[10] = disk->Sides() - 1;
	
	byte* dst = buf + 16;
	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			eUdi::eTrack& t = disk->Track(i, j);
			*dst++ = 0;
			*dst++ = (t.data_len >> 0) & 0xff;
			*dst++ = (t.data_len >> 8) & 0xff;
			int len = t.data_len + t.data_len / 8 + ((t.data_len & 7) ? 1 : 0);
			memcpy(dst, t.data, len);
			dst += len;
		}
	}
	size_t data_len = dst - buf;
	buf[4] = (data_len >> 0) & 0xff;
	buf[5] = (data_len >> 8) & 0xff;
	buf[6] = (data_len >> 16)& 0xff;
	buf[7] = (data_len >> 24)& 0xff;

	int crc = udi_buggy_crc(-1, buf, data_len);
	*dst++ = (crc >> 0) & 0xff;
	*dst++ = (crc >> 8) & 0xff;
	*dst++ = (crc >> 16)& 0xff;
	*dst++ = (crc >> 24)& 0xff;

	size_t full_len = dst - buf;
	bool ok = fwrite(buf, 1, full_len, file) == full_len;
	SAFE_DELETE_ARRAY(buf);
	return ok;
}
