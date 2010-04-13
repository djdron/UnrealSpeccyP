#include "../std.h"
#include "render.h"
#include "../platform/io.h"

namespace xRender
{

static dword screen[WIDTH*HEIGHT];
dword* Screen() { return screen; }

static struct eInit
{
	eInit() { memset(screen, 0, sizeof(screen)); }
}init;

static const eRGBAColor COLOR_WHITE(0x08ffffff);

inline void DrawPixel(const ePoint& p, const eRGBAColor& c)
{
	dword* dst = &screen[p.y*WIDTH + p.x];
	*dst = c.rgba;
}
inline void GetPixel(const ePoint& p, eRGBAColor* c)
{
	dword* dst = &screen[p.y*WIDTH + p.x];
	c->rgba = *dst;
}

void DrawRect(const eRect& r, const eRGBAColor& c)
{
	ePoint p;
	for(p.y = r.top; p.y < r.bottom; ++p.y)
	{
		for(p.x = r.left; p.x < r.right; ++p.x)
		{
			DrawPixel(p, c);
		}
	}
}

void DrawRect(const eRect& r, const eRGBAColor& c, const eRGBAColor& key)
{
	ePoint p;
	eRGBAColor src;
	for(p.y = r.top; p.y < r.bottom; ++p.y)
	{
		for(p.x = r.left; p.x < r.right; ++p.x)
		{
			GetPixel(p, &src);
			if(src.rgba != key.rgba)
			{
				DrawPixel(p, c);
			}
		}
	}
}

static class eFont
{
public:
	eFont() { Clear(self); }
	~eFont() { SAFE_DELETE(data); }
	void Create(int _w, int _h, const char* fname)
	{
		assert(!data);
		w = _w; h = _h;
		FILE* f = fopen(xIo::ResourcePath(fname), "rb");
		assert(f);
		int size = 8 * 256;
		data = new byte[size];
		fread(data, 1, size, f);
		fclose(f);
	}
	void Draw(int _char, const ePoint& p)
	{
		int pitch = 256;
		byte* src = data + _char + pitch*1;
		for(int y = 0; y < h; ++y)
		{
			for(int x = 0; x < w; ++x)
			{
				if(!((*src << x)&0x80))
					continue;
				DrawPixel(ePoint(p.x + x, p.y + y), COLOR_WHITE);
			}
			src += pitch;
		}
	}

public:
	int w, h;
	byte* data;
}font;

void DrawText(const eRect& r, const char* s)
{
	strupr(const_cast<char*>(s));
	ePoint p = r.beg;
	for(int i = 0; s[i]; ++i)
	{
		if(p.x + font.w > r.right)
			break;
		font.Draw(s[i], p);
		p.x += font.w;
	}
}

void _CreateFont(int _w, int _h, const char* fname) { font.Create(_w, _h, fname); }
int FontHeight() { return font.h; }

}
//namespace xRender