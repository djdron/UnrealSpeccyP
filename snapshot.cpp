#include "std.h"

#include "snapshot.h"
#include "z80.h"
#include "memory.h"
#include "speccy.h"

static struct eSnapshot48
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
	byte page0[eMemory::PAGE_SIZE];	// C000-FFFF
} snapshot48;

//=============================================================================
//	eSnapshot::Load
//-----------------------------------------------------------------------------
void eSnapshot::Load(const char* path)
{
	FILE* f = fopen(path, "rb");
	assert(f);
	size_t s = fread(&snapshot48, 1, sizeof(snapshot48), f);
	assert(s < sizeof(snapshot48));
	fclose(f);

	eSnapshot48* hdr = &snapshot48;
	cpu->alt.af = hdr->alt_af; cpu->alt.bc = hdr->alt_bc;
	cpu->alt.de = hdr->alt_de; cpu->alt.hl = hdr->alt_hl;
	cpu->af = hdr->af; cpu->bc = hdr->bc; cpu->de = hdr->de; cpu->hl = hdr->hl;
	cpu->ix = hdr->ix; cpu->iy = hdr->iy; cpu->sp = hdr->sp;
	cpu->i = hdr->i; cpu->r_low = hdr->r; cpu->r_hi = hdr->r & 0x80; cpu->im = hdr->im;
	cpu->iff1 = hdr->iff1 ? 1 : 0;
	devices.IoWrite(0xfe, hdr->pFE);
	memcpy(memory.Get(eMemory::PAGE_SIZE * eMemory::P_RAM0), hdr->page5, eMemory::PAGE_SIZE);
	memcpy(memory.Get(eMemory::PAGE_SIZE * eMemory::P_RAM1), hdr->page2, eMemory::PAGE_SIZE);
	memcpy(memory.Get(eMemory::PAGE_SIZE * eMemory::P_RAM2), hdr->page0, eMemory::PAGE_SIZE);
	cpu->pc = memory.Read(cpu->sp) + 0x100 * memory.Read(cpu->sp+1);
	cpu->sp += 2;
}
