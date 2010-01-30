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
	byte Read(word addr) const;
	void Write(word addr, byte v);
	byte* Get(int page) { return memory + page * PAGE_SIZE; }

	enum ePage
	{
		P_ROM0 = 0, P_ROM1, P_ROM2, P_ROM3,
		P_RAM0, P_RAM1, P_RAM2, P_RAM3,
		P_RAM4, P_RAM5, P_RAM6, P_RAM7,
		P_AMOUNT
	};
	void SetBank(int idx, int page);

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
	eRom(eMemory* m) : memory(m), dos_selected(false) {}
	virtual void Init();
	virtual void Reset();
	virtual void IoWrite(word port, byte v, int tact);
	void Read(word addr);

protected:
	void LoadRom(int page, const char* rom);
	enum
	{
		ROM_128 = eMemory::P_ROM0, ROM_SOS = eMemory::P_ROM1,
		ROM_SYS = eMemory::P_ROM2, ROM_DOS = eMemory::P_ROM3
	};
protected:
	eMemory* memory;
	bool dos_selected;
};

//*****************************************************************************
//	eRam
//-----------------------------------------------------------------------------
class eRam : public eDevice
{
public:
	eRam(eMemory* m) : memory(m) {}
	virtual void Reset();
	virtual void IoWrite(word port, byte v, int tact);
protected:
	eMemory* memory;
};

#endif//__MEMORY_H__
