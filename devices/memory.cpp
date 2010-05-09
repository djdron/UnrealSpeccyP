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

#include "../std.h"
#include "../platform/io.h"
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
	LoadRom(ROM_128, xIo::ResourcePath("res/rom/128_low.rom"));
	LoadRom(ROM_SOS, xIo::ResourcePath("res/rom/sos.rom"));
	LoadRom(ROM_SYS, xIo::ResourcePath("res/rom/service.rom"));
	LoadRom(ROM_DOS, xIo::ResourcePath("res/rom/dos513f.rom"));
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
bool eRom::IoWrite(word port) const
{
	return (!(port & 2) && !(port & 0x8000)); // zx128 port
}
//=============================================================================
//	eRom::IoWrite
//-----------------------------------------------------------------------------
void eRom::IoWrite(word port, byte v, int tact)
{
	page_selected = (page_selected & ~1) + ((v >> 4) & 1);
	memory->SetBank(0, page_selected);
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
bool eRam::IoWrite(word port) const
{
	return (!(port & 2) && !(port & 0x8000)); // zx128 port
}
//=============================================================================
//	eRam::IoWrite
//-----------------------------------------------------------------------------
void eRam::IoWrite(word port, byte v, int tact)
{
	int page = eMemory::P_RAM0 + (v & 7);
	memory->SetBank(3, page);
}
