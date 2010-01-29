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
	virtual void Update();
	virtual void IoWrite(word port, byte v);
	void*	Screen() const { return screen; }
	void	UpdateRay(int tact);

protected:
	void	CreateTables();
	void	CreateTimings();
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
	int		border_add;
	int		border_and;
	byte	border_color;
	bool	first_screen;
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
};

#endif//__ULA_H__
