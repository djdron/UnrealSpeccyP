#include "std.h"

#include "snapshot.h"
#include "z80.h"
#include "memory.h"
#include "speccy.h"

#pragma pack(push, 1)
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
#pragma pack(pop)

struct eZ80Snap : public xZ80::eZ80
{
	void SetState(eSnapshot48* s)
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
		i = s->i;
		r_low = s->r;
		r_hi = s->r & 0x80;
		im = s->im;
		iff1 = s->iff1 ? 1 : 0;
		pc = memory.Read(sp) + 0x100 * memory.Read(sp+1);
		sp += 2;
	}
};

//=============================================================================
//	xSnapshot::Load
//-----------------------------------------------------------------------------
void xSnapshot::Load(eSpeccy* speccy, const char* path)
{
	FILE* f = fopen(path, "rb");
	assert(f);
	size_t r = fread(&snapshot48, 1, sizeof(snapshot48), f);
	assert(r == sizeof(snapshot48));
	fclose(f);

	eSnapshot48* s = &snapshot48;
	devices.IoWrite(0xfe, s->pFE);
	memcpy(memory.Get(eMemory::PAGE_SIZE * eMemory::P_RAM0), s->page5, eMemory::PAGE_SIZE);
	memcpy(memory.Get(eMemory::PAGE_SIZE * eMemory::P_RAM1), s->page2, eMemory::PAGE_SIZE);
	memcpy(memory.Get(eMemory::PAGE_SIZE * eMemory::P_RAM2), s->page0, eMemory::PAGE_SIZE);

	((eZ80Snap*)speccy->CPU())->SetState(s);
}
