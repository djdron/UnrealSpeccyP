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

#ifndef	__MEMORY_H__
#define	__MEMORY_H__

#include "device.h"

#pragma once

//*****************************************************************************
//	eMemory
//-----------------------------------------------------------------------------
class eMemory
{
public:
	eMemory();
	virtual ~eMemory();
	byte Read(word addr) const
	{
		byte* a = bank_read[(addr >> 14) & 3] + (addr & (PAGE_SIZE - 1));
		return *a;
	}
	void Write(word addr, byte v)
	{
		byte* a = bank_write[(addr >> 14) & 3];
		if(!a) //rom write prevent
			return;
		a += (addr & (PAGE_SIZE - 1));
		*a = v;
	}
	byte* Get(int page) { return memory + page * PAGE_SIZE; }

	enum ePage
	{
		P_ROM0 = 0, P_ROM1, P_ROM2, P_ROM3,
		P_RAM0, P_RAM1, P_RAM2, P_RAM3,
		P_RAM4, P_RAM5, P_RAM6, P_RAM7,
		P_AMOUNT
	};
	void SetPage(int idx, int page);
	int	Page(int idx);

	enum { BANKS_AMOUNT = 4, PAGE_SIZE = 0x4000, SIZE = P_AMOUNT * PAGE_SIZE };
protected:
	byte* bank_read[BANKS_AMOUNT];
	byte* bank_write[BANKS_AMOUNT];
	byte* memory;
};

//*****************************************************************************
//	eRom
//-----------------------------------------------------------------------------
class eRom : public eDevice
{
public:
	eRom(eMemory* m) : memory(m), page_selected(0), mode_48k(false) {}
	virtual void Init();
	virtual void Reset();
	virtual bool IoWrite(word port) const;
	virtual void IoWrite(word port, byte v, int tact);
	void Read(word addr)
	{
		byte pc_h = addr >> 8;
		if(page_selected == ROM_SOS && (pc_h == 0x3d))
		{
			page_selected = ROM_DOS;
			memory->SetPage(0, page_selected);
		}
		else if(DosSelected() && (pc_h & 0xc0)) // pc > 0x3fff closes tr-dos
		{
			page_selected = ROM_SOS;
			memory->SetPage(0, page_selected);
		}
	}
	bool DosSelected() const { return page_selected == ROM_DOS; }
	void Mode48k(bool on) { mode_48k = on; }

	static eDeviceId Id() { return D_ROM; }
	virtual dword IoNeed() const { return ION_WRITE; }
protected:
	void LoadRom(int page, const char* rom);
	enum
	{
		ROM_128 = eMemory::P_ROM0, ROM_SOS = eMemory::P_ROM1,
		ROM_SYS = eMemory::P_ROM2, ROM_DOS = eMemory::P_ROM3
	};
protected:
	eMemory* memory;
	int page_selected;
	bool mode_48k;
};

//*****************************************************************************
//	eRam
//-----------------------------------------------------------------------------
class eRam : public eDevice
{
public:
	eRam(eMemory* m) : memory(m), mode_48k(false) {}
	virtual void Reset();
	virtual bool IoWrite(word port) const;
	virtual void IoWrite(word port, byte v, int tact);
	void Mode48k(bool on) { mode_48k = on; }
	bool Mode48k() const { return mode_48k; }
	static eDeviceId Id() { return D_RAM; }
	virtual dword IoNeed() const { return ION_WRITE; }
protected:
	eMemory* memory;
	bool mode_48k;
};

#endif//__MEMORY_H__
