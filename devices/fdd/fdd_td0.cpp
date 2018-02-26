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

// No ID address field was present for this sector,
// but there is a data field. The sector information in
// the header represents fabricated information.
static const dword TD0_SEC_NO_ID = 0x40;

// This sector's data field is missing; no sector data follows this header.
static const dword TD0_SEC_NO_DATA = 0x20;

// A DOS sector copy was requested; this sector was not allocated.
// In this case, no sector data follows this header.
static const dword TD0_SEC_NO_DATA2 = 0x10;

#pragma pack(push, 1)
struct TTd0Sec
{
    byte c;
    byte h;
    byte s;
    byte n;
    byte flags;
    byte crc;
};
#pragma pack(pop)

unsigned int unpack_lzh(const unsigned char* in, size_t size, unsigned char* out);

//=============================================================================
//	eFdd::ReadTd0
//-----------------------------------------------------------------------------
bool eFdd::ReadTd0(const void* _data, size_t data_size)
{
	const byte* buf = (const byte*)_data;
	if(buf[0] == 't' && buf[1] == 'd')
	{
		// unpack lzh
		const byte* packed_buf = buf + 12;
		size_t packed_size = data_size - 12;
		size_t unpacked_size = 12 + unpack_lzh(packed_buf, packed_size, NULL);
		byte* unpacked_data = new byte[unpacked_size];
		memcpy(unpacked_data, buf, 12);
		unpack_lzh(packed_buf, packed_size, unpacked_data + 12);
		unpacked_data[0] = 'T';
		unpacked_data[1] = 'D';
		bool ok = ReadTd0(unpacked_data, unpacked_size);
		SAFE_DELETE_ARRAY(unpacked_data);
		return ok;
	}

	const byte* start = buf + 12;
	if(buf[7] & 0x80) // coment record
	{
		start += 10;
		start += Word(buf + 14);
	}
	const byte* src = start;
	
	byte sides = (buf[9] == 1 ? 1 : 2);
	byte cyls = 0;
	
	for(;;)
	{
		int ns = src[0];
		if(ns == 0xFF)
			break;
		byte cyl = src[1];
		if(cyl >= cyls)
			cyls = cyl + 1; // PhysTrack
		src += 4; // sizeof(track_rec)
		for(; ns; --ns)
		{
			src += sizeof(TTd0Sec); // sizeof(sec_rec)
			assert(src <= buf + data_size);
			if(src > buf + data_size)
				return false;
			src += Word(src) + 2; // data_len
		}
	}
	if(cyls > eUdi::MAX_CYL)
		return false;
	
	SAFE_DELETE(disk);
	disk = new eUdi(cyls, sides);
	
	src = start;
	for(;;)
	{
		const byte* trkh = src;
		src += 4; // sizeof(track_rec)
		
		if(trkh[0] == 0xFF)
			break;

		Seek(trkh[1], trkh[2]);
		int id_len = Track().data_len / 8 + ((Track().data_len & 7) ? 1 : 0);
		memset(Track().data, 0, Track().data_len + id_len);
		
		int pos = 0;
		WriteBlock(pos, 0x4e, 80);		//gap4a
		WriteBlock(pos, 0, 12);			//sync
		WriteBlock(pos, 0xc2, 3, true);	//iam
		Write(pos++, 0xfc);

		int ns = trkh[0];
		Track().sectors_amount = ns;

		for(int s = 0; s < ns; ++s)
		{
			TTd0Sec* SecHdr = (TTd0Sec*)src;
			src += sizeof(TTd0Sec); // sizeof(sec_rec)
			byte flags = SecHdr->flags;
			if(flags & (TD0_SEC_NO_ID | TD0_SEC_NO_DATA | TD0_SEC_NO_DATA2)) // skip sectors with no data & sectors without headers
			{
				size_t src_size = Word(src);
				src += 2; // data_len
				src += src_size;
				continue;
			}
			WriteBlock(pos, 0x4e, 40);		//gap1 50 fixme: recalculate gap1 only for non standard formats
			WriteBlock(pos, 0, 12);			//sync
			WriteBlock(pos, 0xa1, 3, true);	//id am
			Write(pos++, 0xfe);

			eUdi::eTrack::eSector& sec = Track().sectors[s];
			sec.id = Track().data + pos;
			Write(pos++, SecHdr->c);
			Write(pos++, SecHdr->h);
			Write(pos++, SecHdr->s);
			Write(pos++, SecHdr->n);
			word crc = Crc(Track().data + pos - 5, 5);
			Write(pos++, crc >> 8);
			Write(pos++, (byte)crc);
			
			WriteBlock(pos, 0x4e, 22);		//gap2
			WriteBlock(pos, 0, 12);			//sync
			WriteBlock(pos, 0xa1, 3, true);	//data am
			Write(pos++, 0xfb);
			sec.data = Track().data + pos;
			byte* dst = sec.data;

			word src_size = Word(src);
			src += 2; // data_len
			const byte* end_packed_data = src + src_size;
			switch(*src++) // Method
			{
			case 0:  // raw sector
				memcpy(dst, src, src_size - 1);
				break;
			case 1:  // repeated 2-byte pattern
				{
					word n = Word(src);
					src += 2;
					for(word i = 0; i < n; ++i)
					{
						*dst++ = src[0];
						*dst++ = src[1];
					}
				}
				break;
			case 2: // RLE block
				do
				{
					switch(*src++)
					{
					case 0: // Zero count means a literal data block
						for(byte s = *src++; s; --s)
							*dst++ = *src++;
						break;
					case 1:    // repeated fragment
						for(byte s = *src++; s; --s)
						{
							*dst++ = src[0];
							*dst++ = src[1];
						}
						src += 2;
						break;
					default:
						src = end_packed_data;
						break;
					}
				} while(src < end_packed_data);
				break;
			default: // error!
				return false;
			}
			src = end_packed_data;
			int len = sec.Len();
			crc = Crc(Track().data + pos - 1, len + 1);
			pos += len;
			Write(pos++, crc >> 8);
			Write(pos++, (byte)crc);
		}
		WriteBlock(pos, 0x4e, Track().data_len - pos - 1); //gap3
	}
	return true;
}

static const word crcTab[256] =
{
	0x0000, 0x97A0, 0xB9E1, 0x2E41, 0xE563, 0x72C3, 0x5C82, 0xCB22,
	0xCAC7, 0x5D67, 0x7326, 0xE486, 0x2FA4, 0xB804, 0x9645, 0x01E5,
	0x032F, 0x948F, 0xBACE, 0x2D6E, 0xE64C, 0x71EC, 0x5FAD, 0xC80D,
	0xC9E8, 0x5E48, 0x7009, 0xE7A9, 0x2C8B, 0xBB2B, 0x956A, 0x02CA,
	0x065E, 0x91FE, 0xBFBF, 0x281F, 0xE33D, 0x749D, 0x5ADC, 0xCD7C,
	0xCC99, 0x5B39, 0x7578, 0xE2D8, 0x29FA, 0xBE5A, 0x901B, 0x07BB,
	0x0571, 0x92D1, 0xBC90, 0x2B30, 0xE012, 0x77B2, 0x59F3, 0xCE53,
	0xCFB6, 0x5816, 0x7657, 0xE1F7, 0x2AD5, 0xBD75, 0x9334, 0x0494,
	0x0CBC, 0x9B1C, 0xB55D, 0x22FD, 0xE9DF, 0x7E7F, 0x503E, 0xC79E,
	0xC67B, 0x51DB, 0x7F9A, 0xE83A, 0x2318, 0xB4B8, 0x9AF9, 0x0D59,
	0x0F93, 0x9833, 0xB672, 0x21D2, 0xEAF0, 0x7D50, 0x5311, 0xC4B1,
	0xC554, 0x52F4, 0x7CB5, 0xEB15, 0x2037, 0xB797, 0x99D6, 0x0E76,
	0x0AE2, 0x9D42, 0xB303, 0x24A3, 0xEF81, 0x7821, 0x5660, 0xC1C0,
	0xC025, 0x5785, 0x79C4, 0xEE64, 0x2546, 0xB2E6, 0x9CA7, 0x0B07,
	0x09CD, 0x9E6D, 0xB02C, 0x278C, 0xECAE, 0x7B0E, 0x554F, 0xC2EF,
	0xC30A, 0x54AA, 0x7AEB, 0xED4B, 0x2669, 0xB1C9, 0x9F88, 0x0828,
	0x8FD8, 0x1878, 0x3639, 0xA199, 0x6ABB, 0xFD1B, 0xD35A, 0x44FA,
	0x451F, 0xD2BF, 0xFCFE, 0x6B5E, 0xA07C, 0x37DC, 0x199D, 0x8E3D,
	0x8CF7, 0x1B57, 0x3516, 0xA2B6, 0x6994, 0xFE34, 0xD075, 0x47D5,
	0x4630, 0xD190, 0xFFD1, 0x6871, 0xA353, 0x34F3, 0x1AB2, 0x8D12,
	0x8986, 0x1E26, 0x3067, 0xA7C7, 0x6CE5, 0xFB45, 0xD504, 0x42A4,
	0x4341, 0xD4E1, 0xFAA0, 0x6D00, 0xA622, 0x3182, 0x1FC3, 0x8863,
	0x8AA9, 0x1D09, 0x3348, 0xA4E8, 0x6FCA, 0xF86A, 0xD62B, 0x418B,
	0x406E, 0xD7CE, 0xF98F, 0x6E2F, 0xA50D, 0x32AD, 0x1CEC, 0x8B4C,
	0x8364, 0x14C4, 0x3A85, 0xAD25, 0x6607, 0xF1A7, 0xDFE6, 0x4846,
	0x49A3, 0xDE03, 0xF042, 0x67E2, 0xACC0, 0x3B60, 0x1521, 0x8281,
	0x804B, 0x17EB, 0x39AA, 0xAE0A, 0x6528, 0xF288, 0xDCC9, 0x4B69,
	0x4A8C, 0xDD2C, 0xF36D, 0x64CD, 0xAFEF, 0x384F, 0x160E, 0x81AE,
	0x853A, 0x129A, 0x3CDB, 0xAB7B, 0x6059, 0xF7F9, 0xD9B8, 0x4E18,
	0x4FFD, 0xD85D, 0xF61C, 0x61BC, 0xAA9E, 0x3D3E, 0x137F, 0x84DF,
	0x8615, 0x11B5, 0x3FF4, 0xA854, 0x6376, 0xF4D6, 0xDA97, 0x4D37,
	0x4CD2, 0xDB72, 0xF533, 0x6293, 0xA9B1, 0x3E11, 0x1050, 0x87F0
};

static word crc16(const byte* buf, size_t size)
{
	word crc = 0;
	while(size--)
		crc = (crc >> 8) ^ crcTab[(crc & 0xff) ^ *buf++];
	return crc;
}

//=============================================================================
//	eFdd::WriteTd0
//-----------------------------------------------------------------------------
bool eFdd::WriteTd0(FILE* file) const
{
	byte hdr[12];
	memset(hdr, 0, 12);
	hdr[0] = 'T';
	hdr[1] = 'D';
	hdr[4] = 21;
	hdr[6] = 2;
	hdr[9] = disk->Sides();
	word crc = crc16(hdr, 10);
	hdr[10] = crc >> 8;
	hdr[11] = (byte)crc;
	if(fwrite(hdr, 1, 12, file) != 12)
		return false;

	byte zero_sec[256];
	memset(zero_sec, 0, 256);

	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			const eUdi::eTrack& t = disk->Track(i, j);
			byte buf[16];
			buf[0] = t.sectors_amount;
			buf[1] = i;
			buf[2] = j;
			buf[3] = crc16(buf, 3) >> 8;
			if(fwrite(buf, 1, 4, file) != 4)
				return false;
			for(int sec = 0; sec < t.sectors_amount; ++sec)
			{
				const eUdi::eTrack::eSector& s = t.sectors[sec];
				assert(s.id);
				buf[0] = s.Cyl();
				buf[1] = s.Side();
				buf[2] = s.Sec();
				buf[3] = s.data ? s.id[eUdi::eTrack::eSector::ID_LEN] : 1;
				const byte* sec_data = s.data ? s.data : zero_sec;
				int sec_data_len = s.data ? s.Len() : 256;
				buf[4] = 0; // flags
				buf[5] = crc16(sec_data, sec_data_len) >> 8;
				word len = sec_data_len + 1;
				buf[6] = (byte)len;
				buf[7] = len >> 8;
				buf[8] = 0; // compression type = none
				if(fwrite(buf, 1, 9, file) != 9)
					return false;
				if(fwrite(sec_data, 1, sec_data_len, file) != sec_data_len)
					return false;
			}
		}
	}
	byte end[4] = { 0xff, 0x00, 0x00, 0x00 };
	if(fwrite(end, 1, 4, file) != 4)
		return false;
	return true;
}
