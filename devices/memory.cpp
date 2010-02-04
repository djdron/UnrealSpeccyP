#include "../std.h"

#include "memory.h"

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
void eMemory::SetBank(int idx, int page)
{
	byte* addr = memory + page * PAGE_SIZE;
	bank_read[idx] = addr;
	bank_write[idx] = idx ? addr : NULL;
}

//=============================================================================
//	eRom::LoadRom
//-----------------------------------------------------------------------------
void eRom::LoadRom(int page, const char* rom)
{
	FILE* f = fopen(rom, "rb");
	assert(f);
	size_t s = fread(memory->Get(page), 1, eMemory::PAGE_SIZE, f);
	assert(s == eMemory::PAGE_SIZE);
	fclose(f);
}
//=============================================================================
//	eRom::Init
//-----------------------------------------------------------------------------
void eRom::Init()
{
	LoadRom(ROM_128, "rom/128_low.rom");
	LoadRom(ROM_SOS, "rom/sos.rom");
	LoadRom(ROM_SYS, "rom/service.rom");
	LoadRom(ROM_DOS, "rom/dos513f.rom");
}
//=============================================================================
//	eRom::Reset
//-----------------------------------------------------------------------------
void eRom::Reset()
{
	page_selected = ROM_SYS;
	memory->SetBank(0, page_selected);
}
//=============================================================================
//	eRom::IoWrite
//-----------------------------------------------------------------------------
void eRom::IoWrite(word port, byte v, int tact)
{
	if(!(port & 2) && !(port & 0x8000)) // zx128 port
	{
		page_selected = (page_selected & ~1) + ((v >> 4) & 1);
		memory->SetBank(0, page_selected);
	}
}
//=============================================================================
//	eRom::Read
//-----------------------------------------------------------------------------
void eRom::Read(word addr)
{
	byte pc_h = addr >> 8;
	if(page_selected == ROM_SOS && (pc_h == 0x3d))
	{
		page_selected = ROM_DOS;
		memory->SetBank(0, page_selected);
	}
	else if(DosSelected() && (pc_h & 0xc0)) // pc > 0x3fff closes tr-dos
	{
		page_selected = ROM_SOS;
		memory->SetBank(0, page_selected);
	}
}

//=============================================================================
//	eRam::Reset
//-----------------------------------------------------------------------------
void eRam::Reset()
{
	memory->SetBank(1, eMemory::P_RAM5);
	memory->SetBank(2, eMemory::P_RAM2);
	memory->SetBank(3, eMemory::P_RAM0);
}
//=============================================================================
//	eRam::IoWrite
//-----------------------------------------------------------------------------
void eRam::IoWrite(word port, byte v, int tact)
{
	if(!(port & 2) && !(port & 0x8000)) // zx128 port
	{
		int page = eMemory::P_RAM0 + (v & 7);
		memory->SetBank(3, page);
	}
}
