#include "std.h"

#include "memory.h"

//=============================================================================
//	eRom::Init
//-----------------------------------------------------------------------------
void eRom::Init()
{
	FILE* f = fopen("rom/sos.rom", "rb");
	assert(f);
	size_t s = fread(memory.Get(eMemory::P_ROM), 1, eMemory::PAGE_SIZE, f);
	assert(s == eMemory::PAGE_SIZE);
	fclose(f);
}
//=============================================================================
//	eRom::Reset
//-----------------------------------------------------------------------------
void eRom::Reset()
{
	memory.SetBank(0, eMemory::P_ROM);
}

//=============================================================================
//	eRam::Reset
//-----------------------------------------------------------------------------
void eRam::Reset()
{
	memory.SetBank(1, eMemory::P_RAM0);
	memory.SetBank(2, eMemory::P_RAM1);
	memory.SetBank(3, eMemory::P_RAM2);
}

eMemory memory;

//=============================================================================
//	eMemory::eMemory
//-----------------------------------------------------------------------------
eMemory::eMemory() : memory(NULL)
{
	memory = new byte[SIZE];
}
//=============================================================================
//	eMemory::~eMemory
//-----------------------------------------------------------------------------
eMemory::~eMemory()
{
	delete[] memory;
}
//=============================================================================
//	eMemory::Read
//-----------------------------------------------------------------------------
byte eMemory::Read(word addr) const
{
	byte* a = bank_read[(addr >> 14) & 3] + (addr & (PAGE_SIZE - 1));
	return *a;
}
//=============================================================================
//	eMemory::Write
//-----------------------------------------------------------------------------
void eMemory::Write(word addr, byte v)
{
	byte* a = bank_write[(addr >> 14) & 3];
	if(!a) //rom write prevent
		return;
	a += (addr & (PAGE_SIZE - 1));
	*a = v;
}
//=============================================================================
//	eMemory::SetBank
//-----------------------------------------------------------------------------
void eMemory::SetBank(int idx, ePage p)
{
	byte* addr = memory + PAGE_SIZE * p;
	bank_read[idx] = addr;
	bank_write[idx] = idx ? addr : 0;
}
