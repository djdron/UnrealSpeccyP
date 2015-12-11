/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../platform/platform.h"

#include "../z80/z80.h"
#include "../devices/memory.h"
#include "../devices/ula.h"
#include "../devices/sound/ay.h"
#include "../speccy.h"
#include "../platform/endian.h"
#include "../tools/stream_memory.h"

#include "snapshot.h"

#ifdef USE_ZIP
#include <zlib.h>
#endif//USE_ZIP

namespace xSnapshot
{

typedef dword	DWORD;
typedef word	WORD;
typedef byte	BYTE;
typedef char	CHAR;

#ifndef USE_BIG_ENDIAN
#define FOURCC(a, b, c, d) ( (dword) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a)) )
#else
#define FOURCC(a, b, c, d) ( (dword) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d)) )
#endif

// structures & defines bellow are taken from Spectaculator's docs
// http://www.spectaculator.com/docs/zx-state/basic_info.shtml

#pragma pack(push, 1)

// Machine identifiers
#define ZXSTMID_16K          0
#define ZXSTMID_48K          1
#define ZXSTMID_128K         2
#define ZXSTMID_PLUS2        3
#define ZXSTMID_PLUS2A       4
#define ZXSTMID_PLUS3        5
#define ZXSTMID_PLUS3E       6
#define ZXSTMID_PENTAGON128  7
#define ZXSTMID_TC2048       8
#define ZXSTMID_TC2068       9
#define ZXSTMID_SCORPION     10
#define ZXSTMID_SE           11
#define ZXSTMID_TS2068       12
#define ZXSTMID_PENTAGON512  13
#define ZXSTMID_PENTAGON1024 14
#define ZXSTMID_NTSC48K      15
#define ZXSTMID_128KE        16

// Flags (chFlags)
#define ZXSTMF_ALTERNATETIMINGS	1

typedef struct _tagZXSTHEADER
{
	DWORD dwMagic;
	BYTE  chMajorVersion;
	BYTE  chMinorVersion;
	BYTE  chMachineId;
	BYTE  chFlags;
} ZXSTHEADER, *LPZXSTHEADER;

// Block Header. Each real block starts
// with this header.
typedef struct _tagZXSTBLOCK
{
	DWORD dwId;
	DWORD dwSize;
} ZXSTBLOCK, *LPZXSTBLOCK;

#define ZXSTZF_EILAST	1
#define ZXSTZF_HALTED	2

typedef struct _tagZXSTZ80REGS
{
	ZXSTBLOCK blk;
	WORD AF, BC, DE, HL;
	WORD AF1, BC1, DE1, HL1;
	WORD IX, IY, SP, PC;
	BYTE I;
	BYTE R;
	BYTE IFF1, IFF2;
	BYTE IM;
	DWORD dwCyclesStart;
	BYTE chHoldIntReqCycles;
	BYTE chFlags;
	WORD wMemPtr;
} ZXSTZ80REGS, *LPZXSTZ80REGS;

typedef struct _tagZXSTSPECREGS
{
	ZXSTBLOCK blk;
	BYTE chBorder;
	BYTE ch7ffd;
	union {
		BYTE ch1ffd;
		BYTE chEff7;
	};
	BYTE chFe;
	BYTE chReserved[4];
} ZXSTSPECREGS, *LPZXSTSPECREGS;

// Ram pages are compressed using Zlib
#define ZXSTRF_COMPRESSED       1

// Standard 16kb Spectrum RAM page
typedef struct _tagZXSTRAMPAGE
{
	ZXSTBLOCK blk;
	WORD wFlags;
	BYTE chPageNo;
	BYTE chData[1];
} ZXSTRAMPAGE, *LPZXSTRAMPAGE;

// AY Block flags
#define ZXSTAYF_FULLERBOX  1
#define ZXSTAYF_128AY      2

// AY Block. Contains the AY register values
typedef struct _tagZXSTAYBLOCK
{
	ZXSTBLOCK blk;
	BYTE chFlags;
	BYTE chCurrentRegister;
	BYTE chAyRegs[16];
} ZXSTAYBLOCK, *LPZXSTAYBLOCK;

#pragma pack(pop)

struct eZ80AccessorSZX : public xZ80::eZ80
{
	bool SetState(xIo::eStreamMemory& is);
	void SetupDevices(bool model48k)
	{
		devices->Get<eRom>()->Mode48k(model48k);
		devices->Get<eRam>()->Mode48k(model48k);
		devices->Get<eUla>()->Mode48k(model48k);
		devices->Init();
	}
};

template<class B> static bool ReadBlock(xIo::eStreamMemory& is, B* b, const ZXSTBLOCK& block, size_t size = 0)
{
	b->blk = block;
	if(!size)
		size = b->blk.dwSize;
	return is.Read((byte*)b + sizeof(b->blk), size) == size;
}

bool eZ80AccessorSZX::SetState(xIo::eStreamMemory& is)
{
	ZXSTHEADER header;
	if(is.Read(&header, sizeof(header)) != sizeof(header))
		return false;
	if(header.dwMagic != FOURCC('Z', 'X', 'S', 'T'))
		return false;
	bool model48k = false;
	switch(header.chMachineId)
	{
	case ZXSTMID_16K:
	case ZXSTMID_48K:
	case ZXSTMID_NTSC48K:
		model48k = true;
		break;
	case ZXSTMID_128K:
	case ZXSTMID_PENTAGON128:
		break;
	default:
		return false;
	}
	SetupDevices(model48k);
	ZXSTBLOCK block;
	while(is.Read(&block, sizeof(block)) == sizeof(block))
	{
		switch(block.dwId)
		{
		case FOURCC('Z', '8', '0', 'R'):
			{
				ZXSTZ80REGS regs;
				if(!ReadBlock(is, &regs, block))
					return false;
				af = SwapWord(regs.AF);
				bc = SwapWord(regs.BC);
				de = SwapWord(regs.DE);
				hl = SwapWord(regs.HL);
				alt.af = SwapWord(regs.AF1);
				alt.bc = SwapWord(regs.BC1);
				alt.de = SwapWord(regs.DE1);
				alt.hl = SwapWord(regs.HL1);

				ix = SwapWord(regs.IX);
				iy = SwapWord(regs.IY);
				sp = SwapWord(regs.SP);
				pc = SwapWord(regs.PC);
				i = regs.I;
				r_low = regs.R;
				r_hi = regs.R & 0x80;
				im = regs.IM;
				iff1 = regs.IFF1;
				iff2 = regs.IFF2;
				t = Dword((const byte*)&regs.dwCyclesStart) % frame_tacts;
				if(regs.wMemPtr)
					memptr = SwapWord(regs.wMemPtr);
			}
			break;
		case FOURCC('S', 'P', 'C', 'R'):
			{
				ZXSTSPECREGS regs;
				if(!ReadBlock(is, &regs, block))
					return false;
				devices->IoWrite(0xfe, (regs.chFe&0x18) | regs.chBorder, t);
				devices->IoWrite(0x7ffd, model48k ? 0x30 : regs.ch7ffd, t);
				if(model48k)
					devices->Get<eRom>()->SelectPage(eRom::ROM_48);
				else
					devices->Get<eRom>()->SelectPage((regs.ch7ffd & 0x10) ? eRom::ROM_128_0 : eRom::ROM_128_1);
			}
			break;
		case FOURCC('R', 'A', 'M', 'P'):
			{
				ZXSTRAMPAGE ram_page;
				if(!ReadBlock(is, &ram_page, block, sizeof(ZXSTRAMPAGE) - sizeof(ZXSTBLOCK) - 1))
					return false;
				byte* buf = new byte[eMemory::PAGE_SIZE];
				bool ok = false;
				if(SwapWord(ram_page.wFlags)&ZXSTRF_COMPRESSED)
				{
#ifdef USE_ZIP
					size_t size = ram_page.blk.dwSize - (sizeof(ZXSTRAMPAGE) - sizeof(ZXSTBLOCK) - 1);
					byte* buf_compr = new byte[size];
					ok = is.Read(buf_compr, size) == size;
					if(ok)
					{
						z_stream zs;
						memset(&zs, 0, sizeof(zs));
						zs.next_in = buf_compr;
						zs.avail_in = size;
						zs.next_out = buf;
						zs.avail_out = eMemory::PAGE_SIZE;
						ok = inflateInit2(&zs, 15) == Z_OK;
						if(ok)
							ok = inflate(&zs, Z_NO_FLUSH) == Z_STREAM_END;
						inflateEnd(&zs);
					}
					SAFE_DELETE_ARRAY(buf_compr);
#endif//USE_ZIP
				}
				else
				{
					size_t size = ram_page.blk.dwSize - (sizeof(ZXSTRAMPAGE) - sizeof(ZXSTBLOCK) - 1);
					ok = size == eMemory::PAGE_SIZE;
					if(ok)
					{
						ok = is.Read(buf, size) == size;
					}
				}
				if(ok && ram_page.chPageNo <= 7)
				{
					memcpy(memory->Get(eMemory::P_RAM0 + ram_page.chPageNo), buf, eMemory::PAGE_SIZE);
				}
				SAFE_DELETE(buf);
				if(!ok)
					return false;
			}
			break;
		case FOURCC('A', 'Y', '\0', '\0'):
			{
				ZXSTAYBLOCK ay_state;
				if(!ReadBlock(is, &ay_state, block))
					return false;
				devices->Get<eAY>()->SetRegs(ay_state.chAyRegs);
				devices->Get<eAY>()->Select(ay_state.chCurrentRegister);
			}
			break;
		default:
			if(is.Seek(block.dwSize, xIo::eStreamMemory::S_CUR) != 0)
				return false;
		}
		if(is.Pos() == is.Size())
			return true;
	}
	return false;
}

bool LoadSZX(eSpeccy* speccy, const void* data, size_t data_size)
{
	xIo::eStreamMemory is(data, data_size);
	is.Open();
	eZ80AccessorSZX* z80 = (eZ80AccessorSZX*)speccy->CPU();
	return z80->SetState(is);
}

}
//namespace xSnapshot
