#ifndef	__ULA_H__
#define	__ULA_H__

#include "device.h"

#pragma once

class eMemory;

//*****************************************************************************
//	eUla
//-----------------------------------------------------------------------------
class eUla : public eDevice
{
public:
	eUla(eMemory* m);
	virtual ~eUla();
	virtual void Init();
	virtual void Reset();
	virtual void IoWrite(word port, byte v);
	byte*	Screen() const { return screen; }
	void	Update();

	enum eScreen { S_WIDTH = 320, S_HEIGHT = 240, SZX_WIDTH = 256, SZX_HEIGHT = 192 };
protected:
	eMemory* memory;
	byte	border_color;
	bool	first_screen;
	byte*	screen;
	int		scrtab[256]; // offset to start of line
	int		atrtab[256]; // offset to start of attribute line
	byte	colortab[256];// map zx attributes to pc attributes
};

#endif//__ULA_H__
