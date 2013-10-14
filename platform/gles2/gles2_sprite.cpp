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
#ifdef USE_GLES2_SPRITE

#include "gles2_sprite.h"
#include "gles2_shader.h"

namespace xPlatform
{

void SetOrtho(float m[4][4], float left, float right, float bottom, float top, float near, float far, float scale_x, float scale_y);

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
	"uniform mediump float u_alpha;							\n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"	gl_FragColor.a = u_alpha;							\n"
	"}														\n";


eGLES2Sprite::eShaderInfo::eShaderInfo(const char* vs, const char* fs) : program(0), a_position(0), a_texcoord(0), u_vp_matrix(0), u_texture(0), u_alpha(0)
{
	program = CreateProgram(vs, fs);
	if(program)
	{
		a_position	= glGetAttribLocation(program,	"a_position");
		a_texcoord	= glGetAttribLocation(program,	"a_texcoord");
		u_vp_matrix	= glGetUniformLocation(program,	"u_vp_matrix");
		u_texture	= glGetUniformLocation(program,	"u_texture");
		u_alpha		= glGetUniformLocation(program,	"u_alpha");
	}
}

eGLES2Sprite::eGLES2Sprite(GLuint _texture, const ePoint& _size) : texture(_texture), size(_size), shader(vertex_shader, fragment_shader)
{
	const GLfloat vertices[] =
	{
		-0.5f, -0.5f, 0.0f,
		+0.5f, -0.5f, 0.0f,
		+0.5f, +0.5f, 0.0f,
		-0.5f, +0.5f, 0.0f,
	};
	ePoint size_pot = NextPot(size);
	float max_u = float(size.x)/size_pot.x, max_v = float(size.y)/size_pot.y;
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

void eGLES2Sprite::Draw(const ePoint& pos, const ePoint& size, float alpha)
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
		glUniform1f(shader.u_alpha, alpha);
	}
	float proj[4][4];
	SetOrtho(proj, -0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f, 1.0f, 1.0f);
	glUniformMatrix4fv(shader.u_vp_matrix, 1, GL_FALSE, &proj[0][0]);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(shader.u_texture, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

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

#endif//USE_GLES2_SPRITE
#endif//USE_GLES2
