/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_GLES2

#include <GLES2/gl2.h>
#include "../platform.h"
#include "gles2_shader.h"
#include "gles2.h"
#include "../../ui/ui.h"

namespace xPlatform
{

static const char* vertex_shader =
	"uniform mat4 u_vp_matrix;								\n"
	"attribute vec4 a_position;								\n"
	"attribute vec2 a_texcoord;								\n"
	"varying mediump vec2 v_texcoord;						\n"
	"void main()											\n"
	"{														\n"
	"	v_texcoord = a_texcoord;							\n"
	"	gl_Position = u_vp_matrix * a_position;				\n"
	"}														\n";

static const char* fragment_shader =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"}														\n";

static const GLfloat vertices[] =
{
	0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
};

static const int TEX_WIDTH = 512;
static const int TEX_HEIGHT = 256;
static const int WIDTH = 320;
static const int HEIGHT = 240;
static const float max_u = (float)WIDTH/TEX_WIDTH;
static const float max_v = (float)HEIGHT/TEX_HEIGHT;

static const GLfloat uvs[] =
{
	0.0f, 0.0f,
	0.0f, max_v,
	max_u, max_v,
	max_u, 0.0f,
};

static const GLushort indices[] =
{
	0, 1, 2,
	0, 2, 3,
};

static const int kVertexCount = 4;
static const int kIndexCount = 6;

class eGLES2Impl : public eGLES2
{
public:
	eGLES2Impl();
	virtual ~eGLES2Impl();
	virtual void Draw(int w, int h);

protected:
	void UpdateScreen(word* scr);

	word buf_video[WIDTH*HEIGHT];
	GLuint shader_program;
	GLint a_position;
	GLint a_texcoord;
	GLint u_vp_matrix;
	GLint u_texture;
	GLuint buffers[3];
	GLuint textures[1];
};

eGLES2Impl::eGLES2Impl() : shader_program(0), a_position(0), a_texcoord(0), u_vp_matrix(0), u_texture(0)
{
	shader_program = CreateProgram(vertex_shader, fragment_shader);
	if(!shader_program)
	  return;
	a_position = glGetAttribLocation(shader_program, "a_position");
	a_texcoord = glGetAttribLocation(shader_program, "a_texcoord");
	u_vp_matrix = glGetUniformLocation(shader_program, "u_vp_matrix");
	u_texture = glGetUniformLocation(shader_program, "u_texture");

	glGenTextures(1, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);

	glGenBuffers(3, buffers);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, kVertexCount * sizeof(GLfloat) * 3, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, kVertexCount * sizeof(GLfloat) * 2, uvs, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, kIndexCount * sizeof(GL_UNSIGNED_SHORT), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
}

eGLES2Impl::~eGLES2Impl()
{
	if(!shader_program)
		return;
	glDeleteBuffers(3, buffers);
	glDeleteProgram(shader_program);
}

static void SetOrtho(GLfloat* m, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far)
{
	memset(m, 0, sizeof(GLfloat) * 4 * 4);
	m[0] = 2.0f/(right - left);
	m[5] = 2.0f/(top - bottom);
	m[10] = -2.0f/(far - near);
	m[12] = -(right + left)/(right - left);
	m[13] = -(top + bottom)/(top - bottom);
	m[14] = -(far + near)/(far - near);
	m[15] = 1;
}

static inline dword BGR565(byte r, byte g, byte b) { return ((r&~7) << 8)|((g&~3) << 3)|(b >> 3); }
static inline dword RGBX(byte r, byte g, byte b) { return (b << 16)|(g << 8)|r; }

static struct eCachedColors
{
	eCachedColors()
	{
		const byte brightness = 200;
		const byte bright_intensity = 55;
		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? brightness + bright_intensity : brightness;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			items[c] = BGR565(r, g, b);
			items_shifted[c] = items[c] << 16;
			items_rgbx[c] = RGBX(r, g, b);
		}
	}
	dword items[16];
	dword items_shifted[16];
	dword items_rgbx[16];
}
color_cache;

void eGLES2Impl::UpdateScreen(word* scr)
{
	byte* data = (byte*)Handler()->VideoData();
#ifdef USE_UI
	byte* data_ui = (byte*)Handler()->VideoDataUI();
	if(data_ui)
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				xUi::eRGBAColor c_ui = xUi::palette[*data_ui++];
				xUi::eRGBAColor c = color_cache.items_rgbx[*data++];
				*scr++ = BGR565((c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
		}
	}
	else
#endif//USE_UI
	{
		dword* scr1 = (dword*)scr;
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320/8; ++x)
			{
				dword c = color_cache.items[*data++];
				*scr1++ = c | color_cache.items_shifted[*data++];
				c = color_cache.items[*data++];
				*scr1++ = c | color_cache.items_shifted[*data++];
				c = color_cache.items[*data++];
				*scr1++ = c | color_cache.items_shifted[*data++];
				c = color_cache.items[*data++];
				*scr1++ = c | color_cache.items_shifted[*data++];
			}
		}
	}
}

void eGLES2Impl::Draw(int _w, int _h)
{
	if(!shader_program)
		return;

	UpdateScreen(buf_video);

	int w = _w;
	int h = _h;
	if(float(w)/h > 4.0f/3.0f)
		w = float(_h)*4/3;
	else
		h = float(_w)*3/4;
	GLint filter = w % 320 ? GL_LINEAR : GL_NEAREST;

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport((_w - w)/2, (_h - h)/2, w, h);

	GLfloat proj[16];
	SetOrtho(proj, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);

	glUseProgram(shader_program);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, buf_video);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(u_texture, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(a_position);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(a_texcoord);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
	glUniformMatrix4fv(u_vp_matrix, 1, GL_FALSE, proj);
	glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glFlush();
}

eGLES2* eGLES2::Create() { return new eGLES2Impl; }

}
//namespace xPlatform

#endif//USE_GLES2
