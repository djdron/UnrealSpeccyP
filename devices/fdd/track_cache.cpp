#include "../../std.h"

#include "track_cache.h"
#include "fdd.h"

const int Z80FQ = 3500000; // todo: #define as (conf.frame*conf.intfq)
const int FDD_RPS = 5; // rotation speed

#define Min(o, p)	(o < p ? o : p)

//=============================================================================
//	eTrackCache::Seek
//-----------------------------------------------------------------------------
void eTrackCache::Seek(eFdd* _d, int _cyl, int _side, eSeekMode _sm)
{
	if((_d == drive) && (_sm == sm) && (cyl == _cyl) && (side == _side))
		return;

	drive = _d; sm = _sm; s = 0;
	cyl = _cyl; side = _side;
	if(cyl >= _d->cyls || !_d->rawdata) { trkd = 0; return; }

	assert(cyl < MAX_CYLS);
	trkd = _d->trkd[cyl][side];
	trki = _d->trki[cyl][side];
	trklen = _d->trklen[cyl][side];
	if (!trklen) { trkd = 0; return; }

	ts_byte = Z80FQ/(trklen*FDD_RPS);
	if (sm == JUST_SEEK) return; // else find sectors

	for(int i = 0; i < trklen - 8; i++) {
		if (trkd[i] != 0xA1 || trkd[i+1] != 0xFE || !TestI(i)) continue;

		if (s == MAX_SEC) assert(0); //errexit("too many sectors");
		eSector* h = &hdr[s++];
		h->id = trkd+i+2; *(dword*)h = *(dword*)h->id;
		h->crc = *(word*)(trkd+i+6);
		h->c1 = (wd93_crc(trkd+i+1, 5) == h->crc);
		h->data = 0; h->datlen = 0;
//      if (h->l > 5) continue; [vv]

		int end = Min(trklen-8, i+8+43); // 43-DD, 30-SD
		for(int j = i+8; j < end; j++) {
			if (trkd[j] != 0xA1 || !TestI(j) || TestI(j+1)) continue;

			if (trkd[j+1] == 0xF8 || trkd[j+1] == 0xFB) {
				h->datlen = 128 << (h->l & 3); // [vv] FD1793 use only 2 lsb of sector size code
				h->data = trkd+j+2;
				h->c2 = (wd93_crc(h->data-1, h->datlen+1) == *(word*)(h->data+h->datlen));
			}
			break;
		}
	}
}
//=============================================================================
//	eTrackCache::Format
//-----------------------------------------------------------------------------
void eTrackCache::Format()
{
	memset(trkd, 0, trklen);
	memset(trki, 0, trklen/8 + ((trklen&7) ? 1:0));

	byte *dst = trkd;

	dword i;
	for(i = 0; i < 80; i++) // gap4a
		*dst++ = 0x4E;
	for(i = 0; i < 12; i++) //sync
		*dst++ = 0;

	for(i = 0; i < 3; i++) // iam
		Write(dst++ - trkd, 0xC2, 1);
	*dst++ = 0xFC;

	for(int is = 0; is < s; is++)
	{
		for(i = 0; i < 40; i++) // gap1 // 50 [vv] // fixme: recalculate gap1 only for non standard formats
			*dst++ = 0x4E;
		for(i = 0; i < 12; i++) // sync
			*dst++ = 0;
		for(i = 0; i < 3; i++) // idam
			Write(dst++ - trkd, 0xA1, 1);
		*dst++ = 0xFE;

		eSector* sechdr = hdr + is;
		*(dword*)dst = *(dword*)sechdr; // c, h, s, n
		dst += 4;

		dword crc = wd93_crc(dst-5, 5); // crc
		if (sechdr->c1 == 1)
			crc = sechdr->crc;
		if (sechdr->c1 == 2)
			crc ^= 0xFFFF;
		*(dword*)dst = crc;
		dst += 2;

		if(sechdr->data)
		{
			for(i = 0; i < 22; i++) // gap2
				*dst++ = 0x4E;
			for(i = 0; i < 12; i++) // sync
				*dst++ = 0;
			for(i = 0; i < 3; i++) // data am
				Write(dst++ - trkd, 0xA1, 1);
			*dst++ = 0xFB;

//			if(sechdr->l > 5) errexit("strange sector"); // [vv]
			dword len = 128 << (sechdr->l & 3); // data
			if(sechdr->data != (byte*)1)
				memcpy(dst, sechdr->data, len);
			else
				memset(dst, 0, len);

			crc = wd93_crc(dst-1, len+1); // crc
			if(sechdr->c2 == 1)
				crc = sechdr->crcd;
			if(sechdr->c2 == 2)
				crc ^= 0xFFFF;
			*(dword*)(dst+len) = crc;
			dst += len+2;
		}
	}
	if(dst > trklen + trkd)
		assert(0);
//		errexit("track too long");
	while (dst < trkd + trklen)
		*dst++ = 0x4E;
}
//=============================================================================
//	eTrackCache::WriteSector
//-----------------------------------------------------------------------------
int eTrackCache::WriteSector(int sec, byte* data)
{
	eSector* h = GetSector(sec);
	if(!h || !h->data) return 0;
	dword sz = h->datlen;
	memcpy(h->data, data, sz);
	*(word*)(h->data+sz) = (word)wd93_crc(h->data-1, sz+1);
	return sz;
}
//=============================================================================
//	eTrackCache::GetSector
//-----------------------------------------------------------------------------
eSector* eTrackCache::GetSector(int sec)
{
	int i; //Alone Coder 0.36.7
	for(i = 0; i < s; i++)
		if(hdr[i].n == sec)
			break;
	if(i == s)
		return NULL;

	if((hdr[i].l & 3) != 1 || hdr[i].c != cyl) // [vv]
		return NULL;
	return &hdr[i];
}
#define SWAP_2(x) ( (((x) & 0xff) << 8) | ((word)(x) >> 8))
// for WD1793 engine
dword wd93_crc(byte* ptr, dword size)
{
	dword crc = 0xCDB4;
	while(size--)
	{
		crc ^= (*ptr++) << 8;
		for(int j = 8; j; j--) // todo: rewrite with pre-calc'ed table
			if((crc *= 2) & 0x10000) crc ^= 0x1021; // bit representation of x^12+x^5+1
	}
//	return crc & 0xFFFF;
	return SWAP_2(crc);
}

/*
static word crcTab[256] =
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
*/
