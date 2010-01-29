#include "../platform.h"

#ifdef USE_GL

#include <GL/glut.h>

namespace xPlatform
{

static dword tex[512*256];

static const byte brightness = 200;
static const byte bright_intensity = 55;

#define RGBX(r, g, b)	((b << 16)|(g << 8)|r)

//=============================================================================
//	DrawGL
//-----------------------------------------------------------------------------
void DrawGL(void* _data)
{
	byte* data = (byte*)_data;
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			byte r, g, b;
			byte c = data[y*320+x];
			byte i = c&8 ? brightness + bright_intensity : brightness;
			b = c&1 ? i : 0;
			r = c&2 ? i : 0;
			g = c&4 ? i : 0;
			dword* p = &tex[y*512+x];
			*p++ = RGBX(r, g ,b);
		}
	}
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	float u = 320.0f/512.0f;
	float v = 240.0f/256.0f;
	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);	glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, v);		glVertex2f(0.0f, 1.0f);
		glTexCoord2f(u, v);			glVertex2f(1.0f, 1.0f);
		glTexCoord2f(u, 0.0f);		glVertex2f(1.0f, 0.0f);
	}
	glEnd();
}

}
//namespace xPlatform

#endif//USE_GL
