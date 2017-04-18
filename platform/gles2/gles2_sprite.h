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

#include "gles2_shader.h"
#include "../../tools/point.h"

namespace xPlatform
{

class eGLES2Sprite
{
public:
	eGLES2Sprite(const ePoint& size, const char* vshader = NULL, const char* fshader = NULL);
	virtual ~eGLES2Sprite();
	void    Draw(GLuint texture, const ePoint& pos, const ePoint& size, float alpha = 1.0f, float scale_x = 1.0f, float scale_y = 1.0f, bool filtering = true) const;
	void    Draw2(GLuint texture, GLuint texture2, const ePoint& pos, const ePoint& size, float alpha = 1.0f, float scale_x = 1.0f, float scale_y = 1.0f, bool filtering = true) const;
	void    SetColor(float r, float g, float b);

private:
	GLuint	buffer_vertices;
	GLuint	buffer_uv;
	GLuint	buffer_indices;

	struct eShaderInfo
	{
		eShaderInfo(const char* vs, const char* fs);
		GLuint program;
		GLint a_position;
		GLint a_texcoord;
		GLint u_vp_matrix;
		GLint u_texture;
		GLint u_texture2;
		GLint u_color;
	};
	eShaderInfo shader;

	float r, g, b;
};

int NextPot(int v);
inline ePoint NextPot(const ePoint& v) { return ePoint(NextPot(v.x), NextPot(v.y)); }

}
//namespace xPlatform

#endif//USE_GLES2

#endif//__GLES2_SPRITE_H__
