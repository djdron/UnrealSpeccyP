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

#ifndef __GLES2_SHADER_H__
#define __GLES2_SHADER_H__

#ifdef USE_GLES2

#ifndef _IOS
#include <GLES2/gl2.h>
#else//_IOS
#include <OpenGLES/ES2/gl.h>
#endif//_IOS

namespace xPlatform
{

GLuint CreateShader(GLenum type, const char* shader_src);
GLuint CreateProgram(const char* vertex_shader_src, const char* fragment_shader_src);

}
//namespace xPlatform

#endif//USE_GLES2

#endif//__GLES2_SHADER_H__
