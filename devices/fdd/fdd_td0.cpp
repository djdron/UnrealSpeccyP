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

#if 0
int FDD::write_td0(FILE *ff)
{
   unsigned char zerosec[256] = { 0 };
   unsigned char td0hdr[12] = { 0 };

   *(unsigned short*)td0hdr = WORD2('T','D');
   td0hdr[4] = 21; td0hdr[6] = 2; td0hdr[9] = (unsigned char)sides;
   if (*dsc) td0hdr[7] = 0x80;
   *(unsigned short*)(td0hdr + 10) = crc16(td0hdr, 10);
   fwrite(td0hdr, 1, 12, ff);
   if (*dsc) {
      unsigned char inf[0x200] = { 0 };
      strcpy((char*)inf+10, dsc);
      unsigned len = strlen(dsc)+1;
      *(unsigned*)(inf+2) = len;
      *(unsigned short*)inf = crc16(inf+2, len+8);
      fwrite(inf, 1, len+10, ff);
   }

   unsigned c; //Alone Coder 0.36.7
   for (/*unsigned*/ c = 0; c < cyls; c++)
      for (unsigned s = 0; s < sides; s++) {
         t.seek(this,c,s,LOAD_SECTORS);
         unsigned char bf[16];
         *bf = t.s;
         bf[1] = c, bf[2] = s;
         bf[3] = (unsigned char)crc16(bf, 3);
         fwrite(bf, 1, 4, ff);
         for (unsigned sec = 0; sec < t.s; sec++) {
            if (!t.hdr[sec].data) { t.hdr[sec].data = zerosec, t.hdr[sec].datlen = 256, t.hdr[sec].l = 1; }
            *(unsigned*)bf = *(unsigned*)&t.hdr[sec];
            bf[4] = 0; // flags
            bf[5] = (unsigned char)crc16(t.hdr[sec].data, t.hdr[sec].datlen);
            *(unsigned short*)(bf+6) = t.hdr[sec].datlen + 1;
            bf[8] = 0; // compression type = none
            fwrite(bf, 1, 9, ff);
            if (fwrite(t.hdr[sec].data, 1, t.hdr[sec].datlen, ff) != t.hdr[sec].datlen) return 0;
         }
      }
   c = WORD4(0xFF,0,0,0);
   if (fwrite(&c, 1, 4, ff) != 4) return 0;
   return 1;
}

#endif//0


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
		delete[] unpacked_data;
		return ok;
	}

	const byte* start = buf + 12;
	if(buf[7] & 0x80) // coment record
	{
		start += 10;
		start += Word(buf + 14);
	}
	const byte* td0_src = start;
	
	byte sides = (buf[9] == 1 ? 1 : 2);
	byte cyls = 0;
	
	for(;;)
	{
		int ns = td0_src[0];
		if(ns == 0xFF)
			break;
		byte cyl = td0_src[1];
		if(cyl >= cyls)
			cyls = cyl + 1; // PhysTrack
		td0_src += 4; // sizeof(track_rec)
		for(; ns; --ns)
		{
			td0_src += sizeof(TTd0Sec); // sizeof(sec_rec)
			assert(td0_src <= buf + data_size);
			if(td0_src > buf + data_size)
				return false;
			td0_src += Word(td0_src) + 2; // data_len
		}
	}
	if(cyls > eUdi::MAX_CYL)
		return false;
	
	SAFE_DELETE(disk);
	disk = new eUdi(cyls, sides);
	
	td0_src = start;
	for(;;)
	{
		const byte* trkh = td0_src;
		td0_src += 4; // sizeof(track_rec)
		
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
			TTd0Sec* SecHdr = (TTd0Sec*)td0_src;
			td0_src += sizeof(TTd0Sec); // sizeof(sec_rec)
			byte flags = SecHdr->flags;
			if(flags & (TD0_SEC_NO_ID | TD0_SEC_NO_DATA | TD0_SEC_NO_DATA2)) // skip sectors with no data & sectors without headers
			{
				size_t src_size = Word(td0_src);
				td0_src += 2; // data_len
				td0_src += src_size;
				continue;
			}
			WriteBlock(pos, 0x4e, 40);		//gap1 50 fixme: recalculate gap1 only for non standard formats
			WriteBlock(pos, 0, 12);			//sync
			WriteBlock(pos, 0xa1, 3, true);	//id am
			Write(pos++, 0xfe);

			eUdi::eTrack::eSector& sec = Sector(s);
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

			word src_size = Word(td0_src);
			td0_src += 2; // data_len
			const byte* end_packed_data = td0_src + src_size;
			switch(*td0_src++) // Method
			{
			case 0:  // raw sector
				memcpy(dst, td0_src, src_size - 1);
				break;
			case 1:  // repeated 2-byte pattern
				{
					word n = Word(td0_src);
					td0_src += 2;
					for(word i = 0; i < n; ++i)
					{
						*dst++ = td0_src[0];
						*dst++ = td0_src[1];
					}
				}
				break;
			case 2: // RLE block
				do
				{
					switch(*td0_src++)
					{
					case 0: // Zero count means a literal data block
						for(byte s = *td0_src++; s; --s)
							*dst++ = *td0_src++;
						break;
					case 1:    // repeated fragment
						for(byte s = *td0_src++; s; --s)
						{
							*dst++ = td0_src[0];
							*dst++ = td0_src[1];
						}
						td0_src += 2;
						break;
					default:
						td0_src = end_packed_data;
						break;
					}
				} while(td0_src < end_packed_data);
				break;
			default: // error!
				return false;
			}
			td0_src = end_packed_data;
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
