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

#ifndef __UI_H__
#define __UI_H__

#include "../std_types.h"
#include "../tools/point.h"

#pragma once

#ifdef USE_SDL
#define USE_UI
#endif//USE_SDL

//#define USE_UI

#ifdef USE_UI

namespace xUi
{

struct eRect
{
	eRect() { left = top = right = bottom = 0; }
	eRect(ePoint p) { left = top = 0; right = p.x; bottom = p.y; }
	eRect(int _l, int _t, int _r, int _b) { left = _l; top = _t; right = _r; bottom = _b; }
	int Width() const { return right - left; }
	int Height() const { return bottom - top; }
	eRect& Move(const ePoint& offs) { left += offs.x; right += offs.x; top += offs.y; bottom += offs.y; return self; }
	ePoint Beg() const { return ePoint(left, top); }
	ePoint End() const { return ePoint(right, bottom); }
	int left, top, right, bottom;
};

enum { WIDTH = 320, HEIGHT = 240 };

struct eRGBAColor
{
	eRGBAColor(dword c = 0) { rgba = c; }
	eRGBAColor(byte _r, byte _g, byte _b, byte _a = 0xff) : r(_r), g(_g), b(_b), a(_a) {}
	eRGBAColor& operator/=(byte v) { r /= v; g /= v; b /= v; return self; }
	union
	{
#ifdef USE_BIG_ENDIAN
		struct { byte a,b,g,r; };
#else//USE_BIG_ENDIAN
		struct { byte r,g,b,a; };
#endif//USE_BIG_ENDIAN
		dword rgba;
	};
};

enum ePalettedColor { COLOR_NONE, COLOR_WHITE, COLOR_CURSOR, COLOR_BACKGROUND, COLOR_FOCUSED, COLOR_PUSHED, COLOR_PUSHED_FOCUSED, PALETTE_SIZE };
extern eRGBAColor palette[];

extern byte screen[];
void DrawRect(const eRect& r, ePalettedColor c);
void DrawRect(const eRect& r, ePalettedColor c, ePalettedColor key);
void DrawText(const eRect& r, const char* src);
void _CreateFont(int _w, int _h, const char* fname);
ePoint FontSize();

}
//namespace xUi

#endif//USE_UI

#endif//__UI_H__
