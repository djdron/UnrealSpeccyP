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
				eUdi::eTrack::eSector& sec = Sector(i);
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
			if(pos > Track().data_len)
			{
				assert(0); //track too long
			}
			WriteBlock(pos, 0x4e, Track().data_len - pos - 1); //gap3
		}
	}
	return true;
}
