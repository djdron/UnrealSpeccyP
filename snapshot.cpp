#include "std.h"
#include "z80/z80.h"
#include "devices/memory.h"

#include "snapshot.h"
#include "speccy.h"

#pragma pack(push, 1)
static struct eSnapshot
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
} snapshot;
#pragma pack(pop)

struct eZ80Snap : public xZ80::eZ80
{
	void SetState(const eSnapshot* s, bool sna48)
	{
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
			return;
		}
		devices->IoWrite(0x7ffd, s->p7FFD, t);
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
	}
};

//=============================================================================
//	xSnapshot::Load
//-----------------------------------------------------------------------------
void xSnapshot::Load(eSpeccy* speccy, const char* path)
{
	FILE* f = fopen(path, "rb");
	assert(f);
	size_t r = fread(&snapshot, 1, sizeof(snapshot), f);
	fclose(f);
	bool sna48 = r == 49179;
	assert(sna48 || r == sizeof(snapshot));
	((eZ80Snap*)speccy->CPU())->SetState(&snapshot, sna48);
}
