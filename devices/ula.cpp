#include "../std.h"

#include "ula.h"
#include "memory.h"

#define Max(o, p)	(o > p ? o : p)
#define Min(o, p)	(o < p ? o : p)

//=============================================================================
//	eUla::eUla
//-----------------------------------------------------------------------------
eUla::eUla(eMemory* m) : memory(m), border_color(0), first_screen(true), base(NULL), zx_base(0)
	, timing(NULL), prev_t(0), colortab(NULL), frame(0)
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
	// pentagon timings
	line_tacts	= 224;
	paper_start	= 17989;
	border_add	= 0;
	border_and	= 0xffffffff;
	prev_t = 0;
	timing = timings;
	colortab = colortab1;
	CreateTables();
	CreateTimings();
}
//=============================================================================
//	eUla::CreateTables
//-----------------------------------------------------------------------------
void eUla::CreateTables()
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
		byte ink	= a & 7;
		byte paper	= (a >> 3) & 7;
		byte bright	= (a >> 6) & 1;
		byte flash	= (a >> 7) & 1;
		if(ink)
			ink |= bright << 3;		// no bright for 0th color
		if(paper)
			paper |= bright << 3;	// no bright for 0th color
		byte c1 = (paper << 4) | ink;
		if(flash)
		{
			byte t = ink;
			ink = paper;
			paper = t;
		}
		byte c2 = (paper << 4) | ink;
		colortab1[a] = c1;
		colortab2[a] = c2;
	}
}
//=============================================================================
//	eUla::CreateTimings
//-----------------------------------------------------------------------------
// each cpu tact ula painted 2pix
// while painted border ula read color value on each 2pix, on paper each 8pix
//-----------------------------------------------------------------------------
void eUla::CreateTimings()
{
	int b_bottom, b_top, b_left, b_right;
	int mid_lines = SZX_HEIGHT, buf_mid = SZX_WIDTH;
	b_top = b_bottom = (S_HEIGHT - mid_lines) / 2;
	b_left = b_right = (S_WIDTH - buf_mid) / 2;

	int scr_width = S_WIDTH;
	int idx = 0;

	timings[idx++].Set(0, eTiming::Z_SHADOW); // to skip non visible area
	int line_t = paper_start - b_top * line_tacts - b_left / 2;
	for(int i = 0; i < b_top; ++i) // top border
	{
		byte* dst = screen + scr_width * i;
		timings[idx++].Set(Max(line_t, 0), eTiming::Z_BORDER, dst);

		int t = Max(line_t + (b_left + buf_mid + b_right) / 2, 0);
		timings[idx++].Set(t, eTiming::Z_SHADOW);
		line_t += line_tacts;
	}
	for(int i = 0; i < mid_lines; ++i) // screen + border
	{
		byte* dst = screen + scr_width * (i + b_top);
		timings[idx++].Set(Max(line_t, 0), eTiming::Z_BORDER, dst);

		int t = Max(line_t + b_left / 2, 0);
		dst = screen + scr_width * (i + b_top) + b_left;
		timings[idx++].Set(t, eTiming::Z_PAPER, dst, scrtab[i], atrtab[i]);

		t = Max(line_t + (b_left + buf_mid) / 2, 0);
		dst = screen + scr_width * (i + b_top) + b_left + buf_mid;
		timings[idx++].Set(t, eTiming::Z_BORDER, dst);

		t = Max(line_t + (b_left + buf_mid + b_right) / 2, 0);
		timings[idx++].Set(t, eTiming::Z_SHADOW);
		line_t += line_tacts;
	}
	for(int i = 0; i < b_bottom; ++i) // bottom border
	{
		byte* dst = screen + scr_width * (i + b_top + mid_lines);
		timings[idx++].Set(Max(line_t, 0), eTiming::Z_BORDER, dst);

		int t = Max(line_t + (b_left + buf_mid + b_right) / 2, 0);
		timings[idx++].Set(t, eTiming::Z_SHADOW);
		line_t += line_tacts;
	}
	timings[idx].Set(0x7fffffff, eTiming::Z_SHADOW); // shadow area rest
}
//=============================================================================
//	eUla::Reset
//-----------------------------------------------------------------------------
void eUla::Reset()
{
	SwitchScreen(true);
}
//=============================================================================
//	eUla::SwitchScreen
//-----------------------------------------------------------------------------
inline void eUla::SwitchScreen(bool first)
{
	first_screen = first;
	int page = first ? eMemory::P_RAM5: eMemory::P_RAM7;
	base = memory->Get(page);
	zx_base = page == eMemory::P_RAM5 ? 0x4000 : 0xc000;
}
//=============================================================================
//	eUla::Write
//-----------------------------------------------------------------------------
void eUla::Write(word addr, byte v, int tact)
{
	UpdateRay(tact);
}
//=============================================================================
//	eUla::IoWrite
//-----------------------------------------------------------------------------
void eUla::IoWrite(word port, byte v, int tact)
{
	if(!(port & 1)) // port 0xfe
	{
		if((v & 7) != border_color)
		{
			UpdateRay(tact);
			border_color = v & 7;
		}
	}
	if(!(port & 2) && !(port & 0x8000)) // zx128 port
	{
		SwitchScreen(!(v & 0x08));
	}
}
//=============================================================================
//	eUla::Update
//-----------------------------------------------------------------------------
void eUla::Update()
{
	UpdateRay(0x7fff0000);
	prev_t = 0;
	timing = timings;
	if(++frame >= 15)
	{
		frame = 0;
		colortab = colortab == colortab1 ? colortab2 : colortab1;
	}
}
//=============================================================================
//	UpdateRay
//-----------------------------------------------------------------------------
inline void eUla::UpdateRay(int tact)
{
	int last_t = (tact + border_add) & border_and;
	int t = prev_t;

	while(t < last_t)
	{
		switch(timing->zone)
		{
		case eTiming::Z_SHADOW:
			t = (timing + 1)->t;
			break;
		case eTiming::Z_BORDER:
			UpdateRayBorder(t, last_t);
			break;
		case eTiming::Z_PAPER:
			UpdateRayPaper(t, last_t);
			break;
		}
		if(t == (timing + 1)->t)
		{
			timing++;
		}
	}
	prev_t = t;
}
//=============================================================================
//	eUla::UpdateRayBorder
//-----------------------------------------------------------------------------
inline void eUla::UpdateRayBorder(int& t, int last_t)
{
	int offs = (t - timing->t) * 2;
	byte* dst = timing->dst + offs;
	int end = Min(last_t, (timing + 1)->t);
	for(; t < end; ++t)
	{
		*dst++ = border_color;
		*dst++ = border_color;
	}
}
//=============================================================================
//	eUla::UpdateRayPaper
//-----------------------------------------------------------------------------
inline void eUla::UpdateRayPaper(int& t, int last_t)
{
	int offs = (t - timing->t) / 4;
	byte* scr = base + timing->scr_offs + offs;
	byte* atr = base + timing->attr_offs + offs;
	byte* dst = timing->dst + offs * 8;
	int end = Min(last_t, (timing + 1)->t);
	for(int i = 0; t < end; ++i)
	{
		byte pix = scr[i];
		byte ink = colortab[atr[i]];
		byte paper = ink >> 4;
		ink &= 0x0f;
		for(int b = 0; b < 8; ++b)
		{
			*dst++ = ((pix << b) & 0x80) ? ink : paper;
		}
		t += 4;
	}
}
//=============================================================================
//	eUla::FlushScreen
//-----------------------------------------------------------------------------
void eUla::FlushScreen()
{
	int page = first_screen ? eMemory::P_RAM5: eMemory::P_RAM7;
	byte* src = memory->Get(page);
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
			byte pix = *(src + scrtab[y] + x);
			byte ink = colortab[*(src + atrtab[y] + x)];
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
