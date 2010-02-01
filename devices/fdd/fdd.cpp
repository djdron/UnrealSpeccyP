#include "../../std.h"

#include "fdd.h"
#include "track_cache.h"

const int MAX_TRACK_LEN = 6250;
const int trdos_interleave = 1;

byte snbuf[655360]; // large temporary buffer (for reading snapshots)

//=============================================================================
//	eFdd::Init
//-----------------------------------------------------------------------------
void eFdd::Init()
{
	FILE* f = fopen("images/disk2.trd", "rb");
	assert(f);
	size_t r = fread(&snbuf, 1, sizeof(snbuf), f);
	fclose(f);
	if(r == 655360)
		ReadTrd();
	if(!memcmp(snbuf, "SINCLAIR", 8) && (int)r >= 9+(0x100+14)*snbuf[8])
		ReadScl();
}
//=============================================================================
//	eFdd::Free
//-----------------------------------------------------------------------------
void eFdd::Free()
{
	if(rawdata)
	{
		delete[] rawdata;
	}
	memset(this, 0, sizeof(eFdd));
//	conf.trdos_wp[index()] = 0;
//	comp.wd.trkcache.clear();
	t.Clear();
}
//=============================================================================
//	eFdd::NewDisk
//-----------------------------------------------------------------------------
void eFdd::NewDisk(int _cyls, int _sides)
{
	Free();

	cyls = _cyls; sides = _sides;
	unsigned len = MAX_TRACK_LEN;
	unsigned len2 = len + (len/8) + ((len & 7) ? 1 : 0);
	rawsize = cyls * sides * len2;
	rawdata = new byte[rawsize];
	memset(rawdata, 0, rawsize);

	for(unsigned i = 0; i < cyls; ++i)
	{
		for(unsigned j = 0; j < sides; ++j)
		{
			trklen[i][j] = len;
			trkd[i][j] = rawdata + len2 * (i * sides + j);
			trki[i][j] = trkd[i][j] + len;
		}
	}
}
//=============================================================================
//	eFdd::FormatTrd
//-----------------------------------------------------------------------------
void eFdd::FormatTrd()
{
	static const byte lv[3][16] =
	{
		{ 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 },
		{ 1,9,2,10,3,11,4,12,5,13,6,14,7,15,8,16 },
		{ 1,12,7,2,13,8,3,14,9,4,15,10,5,16,11,6 }
	};

	NewDisk(MAX_CYLS, 2);

	for(int c = 0; c < cyls; c++)
	{
		for(int side = 0; side < 2; side++)
		{
			t.Seek(this, c, side, JUST_SEEK); t.s = 16;
			for(int sn = 0; sn < 16; sn++)
			{
				unsigned s = lv[trdos_interleave][sn];
				t.hdr[sn].n = s, t.hdr[sn].l = 1;
				t.hdr[sn].c = c, t.hdr[sn].s = 0;
				t.hdr[sn].c1 = t.hdr[sn].c2 = 0;
				t.hdr[sn].data = (byte*)1;
			}
			t.Format();
		}
	}
}
//=============================================================================
//	eFdd::EmptyDisk
//-----------------------------------------------------------------------------
void eFdd::EmptyDisk()
{
	FormatTrd();
	t.Seek(this, 0, 0, LOAD_SECTORS);
	eSector* s8 = t.GetSector(9);
	if (!s8) return;
	s8->data[0xE2] = 1;                 // first free track
	s8->data[0xE3] = 0x16;              // 80T,DS
	*(short*)(s8->data+0xE5) = 2544;    // free sec
	s8->data[0xE7] = 0x10;              // trdos flag
	t.WriteSector(9,s8->data);         // update sector CRC
}
//=============================================================================
//	eFdd::AddFile
//-----------------------------------------------------------------------------
bool eFdd::AddFile(byte* hdr, byte* data)
{
	t.Seek(this, 0, 0, LOAD_SECTORS);
	eSector* s8 = t.GetSector(9);
	if (!s8) return 0;
	unsigned len = hdr[13];
	unsigned pos = s8->data[0xE4]*0x10;
	eSector* dir = t.GetSector(1+pos/0x100);
	if (!dir) return false;
	if (*(unsigned short*)(s8->data+0xE5) < len) return false; // disk full
	memcpy(dir->data + (pos & 0xFF), hdr, 14);
	*(short*)(dir->data + (pos & 0xFF) + 14) = *(short*)(s8->data+0xE1);
	t.WriteSector(1+pos/0x100,dir->data);

	pos = s8->data[0xE1] + 16*s8->data[0xE2];
	s8->data[0xE1] = (pos+len) & 0x0F, s8->data[0xE2] = (pos+len) >> 4;
	s8->data[0xE4]++;
	*(unsigned short*)(s8->data+0xE5) -= len;
	t.WriteSector(9,s8->data);

	// goto next track. s8 become invalid
	for (unsigned i = 0; i < len; i++, pos++) {
		t.Seek(this, pos/32, (pos/16) & 1, LOAD_SECTORS);
		if (!t.trkd) return false;
		if (!t.WriteSector((pos&0x0F)+1,data+i*0x100)) return 0;
	}
	return true;
}
//=============================================================================
//	eFdd::ReadScl
//-----------------------------------------------------------------------------
bool eFdd::ReadScl()
{
	EmptyDisk();
	int size, i;
	for(i = size = 0; i < snbuf[8]; i++)
		size += snbuf[9 + 14*i + 13];
	if(size > 2544)
	{
		t.Seek(this, 0, 0, LOAD_SECTORS);
		eSector* s8 = t.GetSector(9);
		*(short*)(s8->data+0xE5) = size;    // free sec
		t.WriteSector(9, s8->data);         // update sector CRC
	}
	byte *data = snbuf + 9 + 14*snbuf[8];
	for(i = 0; i < snbuf[8]; i++)
	{
		if(!AddFile(snbuf + 9 + 14*i, data))
			return false;
		data += snbuf[9 + 14*i + 13]*0x100;
	}
	return true;
}
//=============================================================================
//	eFdd::ReadTrd
//-----------------------------------------------------------------------------
bool eFdd::ReadTrd()
{
	FormatTrd();
	for(int i = 0; i < sizeof(snbuf)/*snapsize*/; i += 0x100)
	{
		t.Seek(this, i>>13, (i>>12) & 1, LOAD_SECTORS);
		t.WriteSector(((i>>8) & 0x0F)+1, snbuf+i);
	}
	return true;
}
