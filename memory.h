#ifndef	__MEMORY_H__
#define	__MEMORY_H__

#include "device.h"

#pragma once

//*****************************************************************************
//	eRom
//-----------------------------------------------------------------------------
class eRom : public eDevice
{
public:
	eRom();
	virtual ~eRom();
	virtual void Init();
	virtual bool Read(word addr, byte* v) const;

	enum { ROM_START = 0, ROM_SIZE = 0x4000 };
protected:
	byte* items;
};

//*****************************************************************************
//	eRam
//-----------------------------------------------------------------------------
class eRam : public eDevice
{
public:
	eRam();
	virtual ~eRam();
	virtual bool Read(word addr, byte* v) const;
	virtual void Write(word addr, byte v);

	enum { RAM_START = 0x5b00, RAM_SIZE = 0xa500 };
protected:
	byte* items;
};

#endif//__MEMORY_H__
