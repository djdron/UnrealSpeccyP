/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "gles2_shader.h"
#include "gles2.h"
#include "../platform.h"
#include "../../tools/options.h"
#include "../../ui/ui.h"

namespace xPlatform
{

static struct eOptionZoom : public xOptions::eOptionInt
{
	eOptionZoom() { Set(2); }
	virtual const char* Name() const { return "zoom"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "fill screen", "small border", "no border", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(0, 4, next);
	}
	virtual int Order() const { return 35; }
	float Zoom() const
	{
		switch(self)
		{
		case 2: return 300.0f/256.0f;
		case 3: return 320.0f/256.0f;
		default: return 1.0f;
		}
	}
} op_zoom;

float OpZoom() { return op_zoom.Zoom(); }

static struct eOptionFiltering : public xOptions::eOptionBool
{
	eOptionFiltering() { Set(true); }
	virtual const char* Name() const { return "filtering"; }
	virtual int Order() const { return 36; }
} op_filtering;


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

#ifndef USE_GLES2_SIMPLE_SHADER

static const char* fragment_shader =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"uniform sampler2D u_palette;							\n"
	"void main()											\n"
	"{														\n"
	"	mediump vec4 p0 = texture2D(u_texture, v_texcoord);	\n" // use paletted texture
	"	mediump vec4 c0 = texture2D(u_palette, vec2(p0.r*16.0 + 1.0/16.0*0.5, 0.5)); \n"
	"	gl_FragColor = c0;									\n"
	"}														\n";

static const char* fragment_shader_filtering =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"uniform sampler2D u_palette;							\n"
	"void main()											\n"
	"{														\n"
	"	mediump vec4 p0 = texture2D(u_texture, v_texcoord);	\n" // manually linear filtering of paletted texture is awful
	"	mediump vec4 p1 = texture2D(u_texture, v_texcoord + vec2(1.0/512.0, 0)); 			\n"
	"	mediump vec4 p2 = texture2D(u_texture, v_texcoord + vec2(0, 1.0/256.0)); 			\n"
	"	mediump vec4 p3 = texture2D(u_texture, v_texcoord + vec2(1.0/512.0, 1.0/256.0)); 	\n"
	"	mediump vec4 c0 = texture2D(u_palette, vec2(p0.r*16.0 + 1.0/16.0*0.5, 0.5)); 		\n"
	"	mediump vec4 c1 = texture2D(u_palette, vec2(p1.r*16.0 + 1.0/16.0*0.5, 0.5)); 		\n"
	"	mediump vec4 c2 = texture2D(u_palette, vec2(p2.r*16.0 + 1.0/16.0*0.5, 0.5)); 		\n"
	"	mediump vec4 c3 = texture2D(u_palette, vec2(p3.r*16.0 + 1.0/16.0*0.5, 0.5)); 		\n"
	"	mediump vec2 l = vec2(fract(512.0*v_texcoord.x), fract(256.0*v_texcoord.y)); 		\n"
	"	gl_FragColor = mix(mix(c0, c1, l.x), mix(c2, c3, l.x), l.y); \n"
	"}														\n";

#else//USE_GLES2_SIMPLE_SHADER

static const char* fragment_shader =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"}														\n";

#endif//USE_GLES2_SIMPLE_SHADER

static const GLfloat vertices[] =
{
	-0.5f, -0.5f, 0.0f,
	+0.5f, -0.5f, 0.0f,
	+0.5f, +0.5f, 0.0f,
	-0.5f, +0.5f, 0.0f,
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
	max_u, 0.0f,
	max_u, max_v,
	0.0f, max_v,
};

static const GLushort indices[] =
{
	0, 1, 2,
	0, 2, 3,
};

static const int kVertexCount = 4;
static const int kIndexCount = 6;

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
			items[0][c] = items[1][c] = BGR565(r, g, b);
		}
	}
	static inline word BGR565(byte r, byte g, byte b) { return ((r&~7) << 8)|((g&~3) << 3)|(b >> 3); }
	word items[2][16];
}
color_cache;

class eGLES2Impl : public eGLES2
{
public:
	eGLES2Impl();
	virtual ~eGLES2Impl();
	virtual void Draw(const ePoint& pos, const ePoint& size);

protected:
	struct eShaderInfo
	{
		eShaderInfo(const char* vs, const char* fs) : program(0), a_position(0), a_texcoord(0), u_vp_matrix(0), u_texture(0), u_palette(0)
		{
			program = CreateProgram(vs, fs);
			if(program)
			{
				a_position	= glGetAttribLocation(program,	"a_position");
				a_texcoord	= glGetAttribLocation(program,	"a_texcoord");
				u_vp_matrix	= glGetUniformLocation(program,	"u_vp_matrix");
				u_texture	= glGetUniformLocation(program,	"u_texture");
				u_palette	= glGetUniformLocation(program,	"u_palette");
			}
		}
		GLuint program;
		GLint a_position;
		GLint a_texcoord;
		GLint u_vp_matrix;
		GLint u_texture;
		GLint u_palette;
	};
	void DrawQuad(const eShaderInfo& sh, GLuint texture_palette, bool filtering);

	eShaderInfo shader;
#ifndef USE_GLES2_SIMPLE_SHADER
	eShaderInfo shader_filtering;
#else//USE_GLES2_SIMPLE_SHADER
	void UpdateScreenTexture();
#ifdef USE_UI
	void UpdateUiTexture();
#endif//USE_UI
	dword texture_buffer[WIDTH*HEIGHT*4];
#endif//USE_GLES2_SIMPLE_SHADER

	GLuint buffers[3];
	GLuint textures[2];
#ifdef USE_UI
	GLuint textures_ui[2];
#endif//USE_UI
};

eGLES2Impl::eGLES2Impl()
	: shader(vertex_shader, fragment_shader)
#ifndef USE_GLES2_SIMPLE_SHADER
	, shader_filtering(vertex_shader, fragment_shader_filtering)
#endif//USE_GLES2_SIMPLE_SHADER
{
	if(!shader.program)
		return;
#ifndef USE_GLES2_SIMPLE_SHADER
	if(!shader_filtering.program)
		return;
#endif//USE_GLES2_SIMPLE_SHADER

	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
#ifndef USE_GLES2_SIMPLE_SHADER
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEX_WIDTH, TEX_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
#else//USE_GLES2_SIMPLE_SHADER
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
#endif//USE_GLES2_SIMPLE_SHADER

	glBindTexture(GL_TEXTURE_2D, textures[1]); // color palette
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 2, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, color_cache.items);

#ifdef USE_UI
	glGenTextures(2, textures_ui);
	glBindTexture(GL_TEXTURE_2D, textures_ui[0]);
#ifndef USE_GLES2_SIMPLE_SHADER
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, TEX_WIDTH, TEX_HEIGHT, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
#else//USE_GLES2_SIMPLE_SHADER
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#endif//USE_GLES2_SIMPLE_SHADER

	dword ui_pal[2][16];
	memcpy(&ui_pal[0][0], xUi::palette, xUi::PALETTE_SIZE*sizeof(dword));
	memcpy(&ui_pal[1][0], xUi::palette, xUi::PALETTE_SIZE*sizeof(dword));
	glBindTexture(GL_TEXTURE_2D, textures_ui[1]); // ui color palette
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, ui_pal);
#endif//USE_UI

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
	glDisable(GL_DITHER);
}

eGLES2Impl::~eGLES2Impl()
{
	if(shader.program)
		glDeleteProgram(shader.program);
#ifndef USE_GLES2_SIMPLE_SHADER
	if(shader_filtering.program)
		glDeleteProgram(shader_filtering.program);
	else
		return;
#endif//USE_GLES2_SIMPLE_SHADER
	if(!shader.program)
		return;
	glDeleteBuffers(3, buffers);
	glDeleteTextures(2, textures);
#ifdef USE_UI
	glDeleteTextures(2, textures_ui);
#endif//USE_UI
}

void SetOrtho(float m[4][4], float left, float right, float bottom, float top, float near, float far, float scale_x, float scale_y)
{
	memset(m, 0, 4*4*sizeof(float));
	m[0][0] = 2.0f/(right - left)*scale_x;
	m[1][1] = 2.0f/(top - bottom)*scale_y;
	m[2][2] = -2.0f/(far - near);
	m[3][0] = -(right + left)/(right - left);
	m[3][1] = -(top + bottom)/(top - bottom);
	m[3][2] = -(far + near)/(far - near);
	m[3][3] = 1;
}

void eGLES2Impl::DrawQuad(const eShaderInfo& sh, GLuint texture_palette, bool filtering)
{
	glUniform1i(sh.u_texture, 0);
	//@note : GL_LINEAR must be implemented in shader when palette indexes in texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_NEAREST);

#ifndef USE_GLES2_SIMPLE_SHADER
	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_palette);
	glUniform1i(sh.u_palette, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
#endif//USE_GLES2_SIMPLE_SHADER

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(sh.a_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(sh.a_position);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glVertexAttribPointer(sh.a_texcoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(sh.a_texcoord);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);

	glDrawElements(GL_TRIANGLES, kIndexCount, GL_UNSIGNED_SHORT, 0);
}

void eGLES2Impl::Draw(const ePoint& pos, const ePoint& size)
{
	if(!shader.program)
		return;
#ifndef USE_GLES2_SIMPLE_SHADER
	if(!shader_filtering.program)
		return;
#endif//USE_GLES2_SIMPLE_SHADER

	bool filtering = op_filtering;
	float sx, sy;
#ifdef USE_UI
	float sx1, sy1;
	GetScaleWithAspectRatio43(&sx1, &sy1, size.x, size.y);
#endif//USE_UI
	switch(op_zoom)
	{
	case 0:
	    filtering = false;
	    sx = ((float)WIDTH) / size.x;
	    sy = ((float)HEIGHT) / size.y;
		break;
	default:
		GetScaleWithAspectRatio43(&sx, &sy, size.x, size.y);
		break;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(pos.x, pos.y, size.x, size.y);

#ifndef USE_GLES2_SIMPLE_SHADER
	const eShaderInfo& sh = filtering ? shader_filtering : shader;
	filtering = false;
#else//USE_GLES2_SIMPLE_SHADER
	const eShaderInfo& sh = shader;
	UpdateScreenTexture();
#endif//USE_GLES2_SIMPLE_SHADER

	float z = OpZoom();
	float proj[4][4];
	glDisable(GL_BLEND);
	glUseProgram(sh.program);
	SetOrtho(proj, -0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f, sx*z, sy*z);
	glUniformMatrix4fv(sh.u_vp_matrix, 1, GL_FALSE, &proj[0][0]);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
#ifndef USE_GLES2_SIMPLE_SHADER
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, Handler()->VideoData());
#else//USE_GLES2_SIMPLE_SHADER
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture_buffer);
#endif//USE_GLES2_SIMPLE_SHADER
	DrawQuad(sh, textures[1], filtering);

#ifdef USE_UI
	void* data_ui = Handler()->VideoDataUI();
	if(data_ui)
	{
#ifndef USE_GLES2_SIMPLE_SHADER
		const eShaderInfo& sh = shader_filtering;
		filtering = false;
#else//USE_GLES2_SIMPLE_SHADER
		const eShaderInfo& sh = shader;
		filtering = true;
		UpdateUiTexture();
#endif//USE_GLES2_SIMPLE_SHADER
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(sh.program);
		SetOrtho(proj, -0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f, sx1, sy1); // scale 'fill screen'
		glUniformMatrix4fv(sh.u_vp_matrix, 1, GL_FALSE, &proj[0][0]);
		glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_2D, textures_ui[0]);
#ifndef USE_GLES2_SIMPLE_SHADER
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_LUMINANCE, GL_UNSIGNED_BYTE, data_ui);
#else//USE_GLES2_SIMPLE_SHADER
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer);
#endif//
		DrawQuad(sh, textures_ui[1], filtering);
	}
#endif//USE_UI

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//	glFlush();
}

#ifdef USE_GLES2_SIMPLE_SHADER
void eGLES2Impl::UpdateScreenTexture()
{
	byte* src = (byte*)Handler()->VideoData();
	word* dst = (word*)texture_buffer;
	for(int i = WIDTH*HEIGHT; --i >= 0;)
	{
		*dst++ = color_cache.items[0][*src++];
	}
}

#ifdef USE_UI
void eGLES2Impl::UpdateUiTexture()
{
	byte* src = (byte*)Handler()->VideoDataUI();
	dword* dst = (dword*)texture_buffer;
	for(int i = WIDTH*HEIGHT; --i >= 0;)
	{
		*dst++ = xUi::palette[*src++].rgba;
	}
}
#endif//USE_UI
#endif//USE_GLES2_SIMPLE_SHADER


eGLES2* eGLES2::Create() { return new eGLES2Impl; }

}
//namespace xPlatform

#endif//USE_GLES2
