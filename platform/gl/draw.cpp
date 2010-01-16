#include "../platform.h"

#ifdef USE_GL

#include <GL/gl.h>
#include <math.h>

namespace xPlatform
{

static byte tex[320*240*4];

void DrawGL(byte* data)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			byte r, g, b;
			r = g = b = data[y*320+x];
			byte* p = &tex[y*320*4+x*4];
			*p++ = r;
			*p++ = g;
			*p++ = b;
			*p++ = 0;
		}
	}
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 320, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glColor3f(1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);	glVertex2f(0.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f);	glVertex2f(0.0f, 1.0f);
		glTexCoord2f(1.0f, 1.0f);	glVertex2f(1.0f, 1.0f);
		glTexCoord2f(1.0f, 0.0f);	glVertex2f(1.0f, 0.0f);
	}
	glEnd();
}

}
//namespace xPlatform

#endif//USE_GL
