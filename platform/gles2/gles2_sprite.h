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

#ifndef __GLES2_SPRITE_H__
#define __GLES2_SPRITE_H__

#pragma once

#ifdef USE_GLES2
#ifdef USE_GLES2_SPRITE

#include "gles2_shader.h"
#include "../../tools/point.h"

namespace xPlatform
{

class eGLES2Sprite
{
public:
	eGLES2Sprite(GLuint texture, const ePoint& size);
	virtual ~eGLES2Sprite();
	virtual void Draw(const ePoint& pos, const ePoint& size, float alpha = 1.0f);

private:
	GLuint	texture;
	GLuint	buffer_vertices;
	GLuint	buffer_uv;
	GLuint	buffer_indices;
	ePoint	size;

	struct eShaderInfo
	{
		eShaderInfo(const char* vs, const char* fs);
		GLuint program;
		GLint a_position;
		GLint a_texcoord;
		GLint u_vp_matrix;
		GLint u_texture;
		GLint u_alpha;
	};
	eShaderInfo shader;
};

int NextPot(int v);
inline ePoint NextPot(const ePoint& v) { return ePoint(NextPot(v.x), NextPot(v.y)); }

}
//namespace xPlatform

#endif//USE_GLES2_SPRITE
#endif//USE_GLES2

#endif//__GLES2_SPRITE_H__
