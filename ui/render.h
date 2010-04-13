#ifndef __RENDER_H__
#define __RENDER_H__

#include "../std_types.h"

#pragma once

#define self *this
template<class T> void Clear(T& c) { memset(&c, 0, sizeof(T)); }

struct ePoint
{
	ePoint() : x(0), y(0) {}
	ePoint(int _x, int _y) : x(_x), y(_y) {}
	ePoint& operator+=(ePoint p) { x += p.x; y += p.y; return self; }
	int x, y;
};

struct eRect
{
	eRect() { Clear(self); }
	eRect(ePoint beg) { Clear(self); left = beg.x; top = beg.y; }
	eRect(int _l, int _t, int _r, int _b) { Clear(self); left = _l; top = _t; right = _r; bottom = _b; }
	int Width() const { return right - left; }
	int Height() const { return bottom - top; }
	eRect& Move(ePoint offs) { beg += offs; end += offs; return self; }
	union
	{
		struct { ePoint beg, end; };
		struct { int left, top, right, bottom; };
	};
};

namespace xRender
{
enum { WIDTH = 320, HEIGHT = 240 };

struct eRGBAColor
{
	eRGBAColor() { Clear(self); }
	eRGBAColor(dword c) { rgba = c; }
	eRGBAColor(byte _r, byte _g, byte _b, byte _a = 0xff) : r(_r), g(_g), b(_b), a(_a) {}
	eRGBAColor& operator/=(byte v) { r /= v; g /= v; b /= v; return self; }
	union
	{
		struct { byte r,g,b,a; };
		dword rgba;
	};
};

dword* Screen();
void DrawRect(const eRect& r, const eRGBAColor& c);
void DrawRect(const eRect& r, const eRGBAColor& c, const eRGBAColor& key);
void BlendRect(const eRect& r, const eRGBAColor& c);
void DrawText(const eRect& r, const char* src);
void _CreateFont(int _w, int _h, const char* fname);
int FontHeight();
}
//namespace xRender

#endif//__RENDER_H__
