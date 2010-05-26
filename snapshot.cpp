/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "std.h"
#include "z80/z80.h"
#include "devices/memory.h"

#include "snapshot.h"
#include "speccy.h"

namespace xSnapshot
{

#pragma pack(push, 1)
struct eSnapshot_SNA
{
	byte i;
	word alt_hl, alt_de, alt_bc, alt_af;
	word hl, de, bc, iy, ix;
	byte iff1; // 00 - reset, FF - set
	byte r;
	word af, sp;
	byte im, pFE;
	byte page5[eMemory::PAGE_SIZE];	// 4000-7FFF
	byte page2[eMemory::PAGE_SIZE];	// 8000-BFFF
	byte page[eMemory::PAGE_SIZE];	// C000-FFFF
	// 128k extension
	word pc;
	byte p7FFD;
	byte trdos;
	byte pages[5 * eMemory::PAGE_SIZE]; // all other pages
};

struct eSnapshot_Z80
{
	byte a,f;
	word bc,hl,pc,sp;
	byte i,r,flags;
	word de,bc1,de1,hl1;
	byte a1,f1;
	word iy,ix;
	byte iff1, iff2, im;
	/* 2.01 extension */
	word len, newpc;
	byte model, p7FFD;
	byte r1,r2, p7FFD_1;
	byte AY[16];
	/* 3.0 extension */
};
#pragma pack(pop)


struct eZ80Snap : public xZ80::eZ80
{
	bool SetState(const eSnapshot_SNA* s, size_t buf_size)
	{
		bool sna48 = buf_size == 49179;
		if(!sna48 && buf_size != sizeof(eSnapshot_SNA))
			return false;
		Reset();
		devices->Reset();

		alt.af = s->alt_af;
		alt.bc = s->alt_bc;
		alt.de = s->alt_de;
		alt.hl = s->alt_hl;
		af = s->af;
		bc = s->bc;
		de = s->de;
		hl = s->hl;
		ix = s->ix;
		iy = s->iy;
		sp = s->sp;
		pc = s->pc;
		i = s->i;
		r_low = s->r;
		r_hi = s->r & 0x80;
		im = s->im;
		iff1 = s->iff1 ? 1 : 0;

		devices->IoWrite(0xfe, s->pFE, t);
		int p_size = eMemory::PAGE_SIZE;
		memcpy(memory->Get(eMemory::P_RAM5), s->page5, p_size);
		memcpy(memory->Get(eMemory::P_RAM2), s->page2, p_size);
		int p = !sna48 ? (s->p7FFD & 7) : 0;
		memcpy(memory->Get(eMemory::P_RAM0 + p), s->page, p_size);

		if(sna48)
		{
			pc = memory->Read(sp) + 0x100 * memory->Read(sp+1);
			sp += 2;
			memory->SetBank(0, eMemory::P_ROM1);
			return true;
		}
		devices->IoWrite(0x7ffd, s->p7FFD, t);
		memory->SetBank(0, (s->p7FFD & 0x10) ? eMemory::P_ROM1 : eMemory::P_ROM0);
		const byte* page = s->pages;
		byte mapped = 0x24 | (1 << (s->p7FFD & 7));
		for(int i = 0; i < 8; ++i)
		{
			if(!(mapped & (1 << i)))
			{
				memcpy(memory->Get(eMemory::P_RAM0 + i), page, p_size);
				page += p_size;
			}
		}
		return true;
	}
	bool SetState(const eSnapshot_Z80* s, size_t buf_size)
	{
		Reset();
		devices->Reset();

		bool model48k = (s->model < 3);
		byte flags = s->flags;
		if(flags == 0xFF)
			flags = 1;
		byte* ptr = (byte*)s + 30;
		word reg_pc = s->pc;
		if(reg_pc == 0)
		{ // 2.01
			ptr += 2 + s->len;
			reg_pc = s->newpc;
			memset(memory->Get(eMemory::P_RAM0), 0, eMemory::PAGE_SIZE*8); // clear 128k - first 8 pages
			while(ptr < (byte*)s + buf_size)
			{
				byte* p48[] =
				{
					0, 0, 0, 0,
					memory->Get(eMemory::P_RAM2), memory->Get(eMemory::P_RAM0), 0, 0,
					memory->Get(eMemory::P_RAM5), 0, 0, 0
				};
				byte* p128[] =
				{
					0, 0, 0, memory->Get(eMemory::P_RAM0),
					memory->Get(eMemory::P_RAM1), memory->Get(eMemory::P_RAM2), memory->Get(eMemory::P_RAM3), memory->Get(eMemory::P_RAM4),
					memory->Get(eMemory::P_RAM5), memory->Get(eMemory::P_RAM6), memory->Get(eMemory::P_RAM7), 0
				};
				word len = ptr[0] | word(ptr[1]) << 8;
				if(ptr[2] > 11)
					return false;
				byte* dstpage = model48k ? p48[ptr[2]] : p128[ptr[2]];
				if(!dstpage)
					return false;
				ptr += 3;
				if(len == 0xFFFF)
					memcpy(dstpage, ptr, len = eMemory::PAGE_SIZE);
				else
					UnpackPage(dstpage, eMemory::PAGE_SIZE, ptr, len);
				ptr += len;
			}
		}
		else
		{
			int len = buf_size - 30;
			byte* mem48 = ptr;
			if(flags&0x20)
			{
				//data in packed format
				mem48 = new byte[3*eMemory::PAGE_SIZE];
				UnpackPage(mem48, 3*eMemory::PAGE_SIZE, ptr, len);
			}
			memcpy(memory->Get(eMemory::P_RAM5), mem48, eMemory::PAGE_SIZE);
			memcpy(memory->Get(eMemory::P_RAM2), mem48 + eMemory::PAGE_SIZE, eMemory::PAGE_SIZE);
			memcpy(memory->Get(eMemory::P_RAM0), mem48 + 2*eMemory::PAGE_SIZE, eMemory::PAGE_SIZE);
			if(flags&0x20)
				delete[] mem48;
			model48k = true;
		}
		a = s->a, f = s->f;
		bc = s->bc, de = s->de, hl = s->hl;
		alt.bc = s->bc1, alt.de = s->de1, alt.hl = s->hl1;
		alt.a = s->a1, alt.f = s->f1;
		pc = reg_pc, sp = s->sp; ix = s->ix, iy = s->iy;
		i = s->i, r_low = s->r & 0x7F;
		r_hi = ((flags & 1) << 7);
		byte pFE = (flags >> 1) & 7;
		devices->IoWrite(0xfe, pFE, t);
		iff1 = s->iff1, iff2 = s->iff2; im = s->im & 3;
		devices->IoWrite(0x7ffd, s->p7FFD, t);
		memory->SetBank(0, (model48k || (s->p7FFD & 0x10)) ? eMemory::P_ROM1 : eMemory::P_ROM0);
		return true;
	}
	void UnpackPage(byte* dst, int dstlen, byte* src, int srclen)
	{
		memset(dst, 0, dstlen);
		while(srclen > 0 && dstlen > 0)
		{
			if(srclen >= 4 && src[0] == 0xED && src[1] == 0xED)
			{
				for(byte i = src[2]; i; i--)
					*dst++ = src[3], dstlen--;
				srclen -= 4;
				src += 4;
			}
			else
			{
				*dst++ = *src++;
				--dstlen;
				--srclen;
			}
		}
	}
};

bool Load(eSpeccy* speccy, const char* file)
{
	int l = strlen(file);
	if(l < 4)
		return false;
	FILE* f = fopen(file, "rb");
	if(!f)
		return false;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	byte* buf = new byte[size];
	size_t r = fread(buf, 1, size, f);
	fclose(f);
	if(r != size)
	{
		delete[] buf;
		return false;
	}
	bool ok = false;
	const char* ext = file + l - 4;
	eZ80Snap* z80 = (eZ80Snap*)speccy->CPU();
	if(!strcmp(ext, ".sna") || !strcmp(ext, ".SNA"))
		ok = z80->SetState((const eSnapshot_SNA*)buf, size);
	else if(!strcmp(ext, ".z80") || !strcmp(ext, ".Z80"))
		ok = z80->SetState((const eSnapshot_Z80*)buf, size);
	delete[] buf;
	return ok;
}

}
//namespace xSnapshot
