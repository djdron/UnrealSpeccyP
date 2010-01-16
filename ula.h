#ifndef	__ULA_H__
#define	__ULA_H__

#include "device.h"

#pragma once

//*****************************************************************************
//	eUla
//-----------------------------------------------------------------------------
class eUla : public eDevice
{
public:
	eUla();
	virtual ~eUla();
	virtual void Init();
	virtual void IoWrite(dword port, byte v);
	byte*	Screen() const { return screen; }
	void	Update();

	enum eVram { VRAM_START = 0x4000, VRAM_SIZE = 0x1b00 };
	enum eScreen { S_WIDTH = 320, S_HEIGHT = 240, SZX_WIDTH = 256, SZX_HEIGHT = 192 };
protected:
	byte	border_color;
	byte*	screen;
	int		scrtab[256]; // offset to start of line
	int		atrtab[256]; // offset to start of attribute line
	byte	colortab[256];// map zx attributes to pc attributes
};

#endif//__ULA_H__
