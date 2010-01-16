#include "std.h"

#include "ula.h"

//=============================================================================
//	eUla::eUla
//-----------------------------------------------------------------------------
eUla::eUla() : vram(NULL), border_color(0), screen(NULL)
{
	vram = new byte[VRAM_SIZE];
	screen = new byte[S_WIDTH * S_HEIGHT];
}
//=============================================================================
//	eUla::~eUla
//-----------------------------------------------------------------------------
eUla::~eUla()
{
	delete vram;
	delete screen;
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
void eUla::IoWrite(word port, byte v)
{
	if(!port&1) // port #FE
	{
		border_color = v&7;
	}
}
//=============================================================================
//	eUla::Read
//-----------------------------------------------------------------------------
bool eUla::Read(word addr, byte* v) const
{
	if(addr >= VRAM_START && addr < VRAM_SIZE)
	{
		*v = vram[addr];
		return true;
	}
	return false;
}
//=============================================================================
//	eUla::Write
//-----------------------------------------------------------------------------
void eUla::Write(word addr, byte v)
{
	if(addr >= VRAM_START && addr < VRAM_SIZE)
	{
		vram[addr] = v;
	}
}
//=============================================================================
//	eUla::Update
//-----------------------------------------------------------------------------
void eUla::Update()
{
	for(int y = 0; y < SZX_HEIGHT; ++y)
	{
		for(int x = 0; x < SZX_WIDTH / 8; x++)
		{
			byte* dst = screen + y*320;
			word pix = vram[scrtab[y] + x];
			for(int b = 0; b < 8; ++b)
			{
				*dst++ = ((pix << b) & 0x80) ? 255 : 0;
			}
		}
	}
	/*	dword b = border_color * 0x11001100;
	int border_half_width = (S_WIDTH - SZX_WIDTH) / 2;
	int border_half_height = (S_HEIGHT - SZX_HEIGHT) / 2;

	for(int i = border_half_height * S_WIDTH / 4; i; --i)
	{
		*(dword*)dst = b; dst += 4;
	}
	for(int y = 0; y < SZX_HEIGHT; ++y)
	{
		for(int x = border_half_width / 4; x; --x)
		{
			*(dword*)dst = b; dst += 4;
		}
		for(int x = 0; x < SZX_WIDTH / 8; x++)
		{
			*dst++ = vram[scrtab[y] + x];
			*dst++ = colortab[vram[atrtab[y] + x]];
		}
		for(int x = border_half_width / 4; x; --x)
		{
			*(dword*)dst = b; dst += 4;
		}
	}
	for(int i = border_half_height * S_WIDTH / 4; i; --i)
	{
		*(dword*)dst = b; dst += 4;
	}
*/
}
