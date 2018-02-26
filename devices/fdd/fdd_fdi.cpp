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
//	eFdd::ReadFdi
//-----------------------------------------------------------------------------
bool eFdd::ReadFdi(const void* _data, size_t data_size)
{
	const byte* buf = (const byte*)_data;
	SAFE_DELETE(disk);
	disk = new eUdi(buf[4], buf[6]);
	
	const byte* trk = buf + 0x0E + Word(buf + 0x0C);
	const byte* dat = buf + Word(buf + 0x0A);
	
	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			Seek(i, j);
			
			int id_len = Track().data_len / 8 + ((Track().data_len & 7) ? 1 : 0);
			memset(Track().data, 0, Track().data_len + id_len);
			
			int pos = 0;
			WriteBlock(pos, 0x4e, 80);		//gap4a
			WriteBlock(pos, 0, 12);			//sync
			WriteBlock(pos, 0xc2, 3, true);	//iam
			Write(pos++, 0xfc);
			
			const byte* t0 = dat + Dword(trk);
			int ns = trk[6];
			Track().sectors_amount = ns;
			trk += 7;
			for(int i = 0; i < ns; ++i)
			{
				WriteBlock(pos, 0x4e, 40);		//gap1 50 fixme: recalculate gap1 only for non standard formats
				WriteBlock(pos, 0, 12);			//sync
				WriteBlock(pos, 0xa1, 3, true);	//id am
				Write(pos++, 0xfe);
				eUdi::eTrack::eSector& sec = Track().sectors[i];
				sec.id = Track().data + pos;
				Write(pos++, trk[0]);
				Write(pos++, trk[1]);
				Write(pos++, trk[2]);
				Write(pos++, trk[3]);
				word crc = Crc(Track().data + pos - 5, 5);
				Write(pos++, crc >> 8);
				Write(pos++, (byte)crc);
				
				if(trk[4] & 0x40)
				{
					sec.data = NULL;
				}
				else
				{
					const byte* data = t0 + Word(trk+5);
					if(data + 128 > buf + data_size)
						return false;
					WriteBlock(pos, 0x4e, 22);		//gap2
					WriteBlock(pos, 0, 12);			//sync
					WriteBlock(pos, 0xa1, 3, true);	//data am
					Write(pos++, 0xfb);
					sec.data = Track().data + pos;
					int len = sec.Len();
					memcpy(sec.data, data, len);
					crc = Crc(Track().data + pos - 1, len + 1);
					if(!(trk[4] & (1<<(trk[3] & 3))))
						crc ^= 0xffff;
					pos += len;
					Write(pos++, crc >> 8);
					Write(pos++, (byte)crc);
				}
				trk += 7;
			}
			assert(pos <= Track().data_len);
			WriteBlock(pos, 0x4e, Track().data_len - pos - 1); //gap3
		}
	}
	return true;
}
//=============================================================================
//	eFdd::WriteFdi
//-----------------------------------------------------------------------------
bool eFdd::WriteFdi(FILE* file) const
{
	int total_sectors = 0;
	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			total_sectors += disk->Track(i, j).sectors_amount;
		}
	}

	size_t desc_offset = 14 + (total_sectors + disk->Cyls() * disk->Sides()) * 7;
	size_t data_offset = desc_offset + 1;
	byte hdr[14] =
	{
		'F', 'D', 'I',
		0,
		(byte)disk->Cyls(), 0,
		(byte)disk->Sides(), 0,
		(byte)desc_offset, byte(desc_offset >> 8),
		(byte)data_offset, byte(data_offset >> 8),
		0, 0,
	};
	if(fwrite(hdr, 1, 14, file) != 14)
		return false;

	size_t trkoffs = 0;
	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			const eUdi::eTrack& t = disk->Track(i, j);
			byte track_hdr[7] =
			{
				byte((trkoffs >> 0) & 0xff),
				byte((trkoffs >> 8) & 0xff),
				byte((trkoffs >> 16)& 0xff),
				byte((trkoffs >> 24)& 0xff),
				0, 0,
				(byte)t.sectors_amount
			};
			if(fwrite(track_hdr, 1, 7, file) != 7)
				return false;
			size_t secoffs = 0;
			for(int se = 0; se < t.sectors_amount; ++se)
			{
				const eUdi::eTrack::eSector& s = t.sectors[se];
				assert(s.id);
				byte sec_hdr[7] =
				{
					(byte)s.Cyl(),
					(byte)s.Side(),
					(byte)s.Sec(),
					(byte)s.id[eUdi::eTrack::eSector::ID_LEN],
					byte(s.data ? (s.data[-1] == 0xf8 ? 0x80 : (1 << (s.id[eUdi::eTrack::eSector::ID_LEN] & 3))) : 0x40),
					byte((secoffs >> 0) & 0xff),
					byte((secoffs >> 8) & 0xff)
				};
				if(fwrite(sec_hdr, 1, 7, file) != 7)
					return false;
				if(s.data)
					secoffs += s.Len();
			}
			trkoffs += secoffs;
		}
	}

	byte desc_eol = 0;
	if(fwrite(&desc_eol, 1, 1, file) != 1)
		return false;

	for(int i = 0; i < disk->Cyls(); ++i)
	{
		for(int j = 0; j < disk->Sides(); ++j)
		{
			const eUdi::eTrack& t = disk->Track(i, j);
			for(int se = 0; se < t.sectors_amount; ++se)
			{
				const eUdi::eTrack::eSector& s = t.sectors[se];
				if(s.data)
				{
					if(fwrite(s.data, 1, s.Len(), file) != s.Len())
						return false;
				}
			}
		}
	}
	return true;
}
