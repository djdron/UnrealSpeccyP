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

#ifdef USE_EMBEDDED_RESOURCES
#include "res/rom/sos128_0.h"
#include "res/rom/sos128_1.h"
#include "res/rom/sos48.h"
#include "res/rom/service.h"
#include "res/rom/dos513f.h"
#endif//USE_EMBEDDED_RESOURCES

#ifdef USE_EXTERN_RESOURCES
extern byte sos128_0[];
extern byte sos128_1[];
extern byte sos48[];
extern byte service[];
extern byte dos513f[];
#endif//USE_EXTERN_RESOURCES

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
//	eMemory::SetPage
//-----------------------------------------------------------------------------
void eMemory::SetPage(int idx, int page)
{
	byte* addr = Get(page);
	bank_read[idx] = addr;
	bank_write[idx] = idx ? addr : NULL;
}
//=============================================================================
//	eMemory::Page
//-----------------------------------------------------------------------------
int	eMemory::Page(int idx)
{
	byte* addr = bank_read[idx];
	for(int p = 0; p < P_AMOUNT; ++p)
	{
		if(Get(p) == addr)
			return p;
	}
	assert(false);
	return -1;
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
#if defined(USE_EMBEDDED_RESOURCES) || defined(USE_EXTERN_RESOURCES)
	memcpy(memory->Get(ROM_128_0),	sos128_0,	eMemory::PAGE_SIZE);
	memcpy(memory->Get(ROM_128_1),	sos128_1,	eMemory::PAGE_SIZE);
	memcpy(memory->Get(ROM_48),		sos48,		eMemory::PAGE_SIZE);
	memcpy(memory->Get(ROM_SYS),	service,	eMemory::PAGE_SIZE);
	memcpy(memory->Get(ROM_DOS),	dos513f,	eMemory::PAGE_SIZE);
#else//USE_EMBEDDED_RESOURCES
	LoadRom(ROM_128_0,	xIo::ResourcePath("res/rom/sos128_0.rom"));
	LoadRom(ROM_128_1,	xIo::ResourcePath("res/rom/sos128_1.rom"));
	LoadRom(ROM_48,		xIo::ResourcePath("res/rom/sos48.rom"));
	LoadRom(ROM_SYS,	xIo::ResourcePath("res/rom/service.rom"));
	LoadRom(ROM_DOS,	xIo::ResourcePath("res/rom/dos513f.rom"));
#endif//USE_EMBEDDED_RESOURCES
}
//=============================================================================
//	eRom::Reset
//-----------------------------------------------------------------------------
void eRom::Reset()
{
	page_selected = mode_48k ? ROM_48 : ROM_SYS;
	memory->SetPage(0, page_selected);
}
//=============================================================================
//	eRom::IoWrite
//-----------------------------------------------------------------------------
bool eRom::IoWrite(word port) const
{
	return !mode_48k && !(port & 2) && !(port & 0x8000); // zx128 port
}
//=============================================================================
//	eRom::IoWrite
//-----------------------------------------------------------------------------
void eRom::IoWrite(word port, byte v, int tact)
{
	page_selected = (page_selected & ~1) + ((v >> 4) & 1);
	memory->SetPage(0, page_selected);
}

//=============================================================================
//	eRam::Reset
//-----------------------------------------------------------------------------
void eRam::Reset()
{
	memory->SetPage(1, eMemory::P_RAM5);
	memory->SetPage(2, eMemory::P_RAM2);
	memory->SetPage(3, eMemory::P_RAM0);
}
//=============================================================================
//	eRam::IoWrite
//-----------------------------------------------------------------------------
bool eRam::IoWrite(word port) const
{
	return !mode_48k && !(port & 2) && !(port & 0x8000); // zx128 port
}
//=============================================================================
//	eRam::IoWrite
//-----------------------------------------------------------------------------
void eRam::IoWrite(word port, byte v, int tact)
{
	int page = eMemory::P_RAM0 + (v & 7);
	memory->SetPage(3, page);
}
