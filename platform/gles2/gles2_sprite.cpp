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

#include "gles2_sprite.h"
#include "../../std.h"

namespace xPlatform
{

static void SetOrtho(float m[4][4], float left, float right, float bottom, float top, float near, float far, float scale_x, float scale_y)
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
	"uniform mediump vec4 u_color;						    \n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"	gl_FragColor *= u_color;						    \n"
	"}														\n";


eGLES2Sprite::eShaderInfo::eShaderInfo(const char* vs, const char* fs) : program(0), a_position(0), a_texcoord(0), u_vp_matrix(0), u_texture(0), u_texture2(0), u_color(0)
{
	program = CreateProgram(vs, fs);
	if(program)
	{
		a_position	= glGetAttribLocation(program,	"a_position");
		a_texcoord	= glGetAttribLocation(program,	"a_texcoord");
		u_vp_matrix	= glGetUniformLocation(program,	"u_vp_matrix");
		u_texture	= glGetUniformLocation(program,	"u_texture");
		u_texture2	= glGetUniformLocation(program,	"u_texture2");
		u_color		= glGetUniformLocation(program,	"u_color");
	}
}

eGLES2Sprite::eGLES2Sprite(const ePoint& _size, const char* vshader, const char* fshader)
	: shader(vshader ? vshader : vertex_shader, fshader ? fshader : fragment_shader)
	, r(1.0f), g(1.0f), b(1.0f)
{
	const GLfloat vertices[] =
	{
		-0.5f, -0.5f, 0.0f,
		+0.5f, -0.5f, 0.0f,
		+0.5f, +0.5f, 0.0f,
		-0.5f, +0.5f, 0.0f,
	};
	ePoint size_pot = NextPot(_size);
	float max_u = float(_size.x)/size_pot.x, max_v = float(_size.y)/size_pot.y;
	const GLfloat uvs[] =
	{
		0.0f, 0.0f,
		max_u, 0.0f,
		max_u, max_v,
		0.0f, max_v,
	};
	const GLushort indices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	glGenBuffers(1, &buffer_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_vertices);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 3, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &buffer_uv);
	glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 2, uvs, GL_STATIC_DRAW);

	glGenBuffers(1, &buffer_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GL_UNSIGNED_SHORT), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

eGLES2Sprite::~eGLES2Sprite()
{
	glDeleteBuffers(1, &buffer_vertices);
	glDeleteBuffers(1, &buffer_uv);
	glDeleteBuffers(1, &buffer_indices);
	glDeleteProgram(shader.program);
}

void eGLES2Sprite::Draw(GLuint texture, const ePoint& pos, const ePoint& size, float alpha, float scale_x, float scale_y, bool filtering) const
{
	glViewport(pos.x, pos.y, size.x, size.y);
	glUseProgram(shader.program);
	if(alpha > 0.999f)
	{
		glDisable(GL_BLEND);
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glUniform4f(shader.u_color, r, g, b, alpha);
	float proj[4][4];
	SetOrtho(proj, -0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f, scale_x, scale_y);
	glUniformMatrix4fv(shader.u_vp_matrix, 1, GL_FALSE, &proj[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(shader.u_texture, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_NEAREST);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_vertices);
	glVertexAttribPointer(shader.a_position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(shader.a_position);

	glBindBuffer(GL_ARRAY_BUFFER, buffer_uv);
	glVertexAttribPointer(shader.a_texcoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(shader.a_texcoord);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_indices);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void eGLES2Sprite::Draw2(GLuint texture, GLuint texture2, const ePoint& pos, const ePoint& size, float alpha, float scale_x, float scale_y, bool filtering) const
{
	glUseProgram(shader.program);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glUniform1i(shader.u_texture2, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	Draw(texture, pos, size, alpha, scale_x, scale_y, filtering);
}

void eGLES2Sprite::SetColor(float _r, float _g, float _b)
{
	r = _r;
	g = _g;
	b = _b;
}

int NextPot(int v)
{
	--v;
	v |= (v >> 1);
	v |= (v >> 2);
	v |= (v >> 4);
	v |= (v >> 8);
	v |= (v >> 16);
	return ++v;
}

}
//namespace xPlatform

#endif//USE_GLES2
