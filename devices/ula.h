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
	virtual void FrameUpdate();
	virtual bool IoRead(word port) const;
	virtual bool IoWrite(word port) const;
	virtual void IoRead(word port, byte* v, int tact);
	virtual void IoWrite(word port, byte v, int tact);
	void	Write(int tact) { if(prev_t < tact) UpdateRay(tact); }

	void*	Screen() const { return screen; }

	byte	BorderColor() const { return border_color; }
	bool	FirstScreen() const { return first_screen; }
	void	Mode48k(bool on)	{ mode_48k = on; }

	static eDeviceId Id() { return D_ULA; }
	virtual dword IoNeed() const { return ION_WRITE|ION_READ; }
protected:
	void	CreateTables();
	void	CreateTimings();
	void	SwitchScreen(bool first, int tact);
	void	UpdateRay(int tact);
	void	UpdateRayBorder(int& t, int last_t);
	void	UpdateRayPaper(int& t, int last_t);
	void	FlushScreen();

	enum eScreen { S_WIDTH = 320, S_HEIGHT = 240, SZX_WIDTH = 256, SZX_HEIGHT = 192 };
	struct eTiming
	{
		enum eZone { Z_SHADOW, Z_BORDER, Z_PAPER };
		void Set(int _t, eZone _zone = Z_SHADOW, byte* _dst = NULL
			, int _scr_offs = 0, int _attr_offs = 0)
		{
			dst			= _dst;
			t			= _t;
			zone		= _zone;
			scr_offs	= _scr_offs;
			attr_offs	= _attr_offs;
		}
		byte*	dst;		// screen raster pointer
		int		t;			// start zone tact
		eZone	zone;		// what are drawing: shadow/border/screen
		int		scr_offs;
		int		attr_offs;
	};
protected:
	eMemory* memory;
	int		line_tacts;		// t-states per line
	int		paper_start;	// start of paper
	byte	border_color;
	bool	first_screen;
	byte*	base;
	byte*	screen;
	int		scrtab[256];	// offset to start of line
	int		atrtab[256];	// offset to start of attribute line
	byte	colortab1[256];	// map zx attributes to pc attributes
	byte	colortab2[256];
	byte*	colortab;

	eTiming	timings[4 * S_HEIGHT];
	eTiming* timing;
	int		prev_t;			// last drawn pixel's tact
	int		frame;
	bool	mode_48k;
};

#endif//__ULA_H__
