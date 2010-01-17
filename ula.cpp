#include "std.h"

#include "ula.h"
#include "memory.h"

//=============================================================================
//	eUla::eUla
//-----------------------------------------------------------------------------
eUla::eUla() : border_color(0), screen(NULL)
{
	screen = new byte[S_WIDTH * S_HEIGHT];
}
//=============================================================================
//	eUla::~eUla
//-----------------------------------------------------------------------------
eUla::~eUla()
{
	delete[] screen;
}
//=============================================================================
//	eUla::Init
//-----------------------------------------------------------------------------
void eUla::Init()
{
	int i = 0; // calc screen addresses
	for(int p = 0; p < 4; p++)
	{
		for(int y = 0; y < 8; y++)
		{
			for(int o = 0; o < 8; o++, i++)
			{
				scrtab[i] = p*0x800 + y*0x20 + o*0x100,
				atrtab[i] = 0x1800 + (p*8+y)*32;
			}
		}
	}

	// make colortab: zx-attr -> pc-attr
	for(int a = 0; a < 0x100; a++)
	{
		byte ink = a & 7;
		byte paper = (a >> 3) & 7;
		byte bright = (a >> 6) & 1;
		byte flash = (a >> 7) & 1;
		if(ink)
			ink |= bright << 3; // no bright for 0th color
		if(paper)
			paper |= bright << 3; // no bright for 0th color
		if(flash)
		{
			byte t = ink;
			ink = paper;
			paper = t;
		}
		byte c = (paper << 4) | ink;
		colortab[a] = c;
	}
}
//=============================================================================
//	eUla::IoWrite
//-----------------------------------------------------------------------------
void eUla::IoWrite(dword port, byte v)
{
	if(!(port & 1)) // port #FE
	{
		border_color = v & 7;
	}
}
//=============================================================================
//	eUla::Update
//-----------------------------------------------------------------------------
void eUla::Update()
{
	byte* dst = screen;
	int border_half_width = (S_WIDTH - SZX_WIDTH) / 2;
	int border_half_height = (S_HEIGHT - SZX_HEIGHT) / 2;

	for(int i = 0; i < border_half_height * S_WIDTH; ++i)
	{
		*dst++ = border_color;
	}
	for(int y = 0; y < SZX_HEIGHT; ++y)
	{
		for(int x = 0; x < border_half_width; ++x)
		{
			*dst++ = border_color;
		}
		for(int x = 0; x < SZX_WIDTH / 8; x++)
		{
			byte pix = memory.Read(VRAM_START + scrtab[y] + x);
			byte ink = colortab[memory.Read(VRAM_START + atrtab[y] + x)];
			byte paper = ink >> 4;
			ink &= 0x0f;
			for(int b = 0; b < 8; ++b)
			{
				*dst++ = ((pix << b) & 0x80) ? ink : paper;
			}
		}
		for(int x = 0; x < border_half_width; ++x)
		{
			*dst++ = border_color;
		}
	}
	for(int i = 0; i < border_half_height * S_WIDTH; ++i)
	{
		*dst++ = border_color;
	}
}
