/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../platform.h"

#ifdef USE_GL

#ifdef _WINDOWS
#include <windows.h>
#endif//_WINDOWS

#ifndef _MAC
#include <GL/gl.h>
#else//_MAC
#include <OpenGL/gl.h>
#endif//_MAC

#ifdef _LINUX
//#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glx.h>
#endif//_LINUX

#ifdef _WINDOWS
#include <GL/wglext.h>
#endif//_WINDOWS

namespace xPlatform
{

#ifdef _LINUX
void VsyncGL(bool on)
{
#ifdef GLX_SGI_swap_control
	static bool inited = false;
	static PFNGLXSWAPINTERVALSGIPROC si = NULL;
	if(!inited)
	{
		si = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalSGI");
		inited = true;
	}
	if(si)
		si(on);
//	const char* exts = (const char*)glGetString(GL_EXTENSIONS);
//	if(strstr(exts, "GLX_SGI_swap_control"))
//	{
//		glXSwapIntervalSGI(on);
//	}
#endif//GLX_SGI_swap_control
}
#endif//_LINUX

#ifdef _WINDOWS
void VsyncGL(bool on)
{
#ifdef WGL_EXT_swap_control
	static bool inited = false;
	static PFNWGLSWAPINTERVALEXTPROC si = NULL;
	if(!inited)
	{
		si = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		inited = true;
	}
	if(si)
		si(on);
#endif//WGL_EXT_swap_control
}
#endif//_WINDOWS
	
#ifdef _MAC
void VsyncGL(bool on)
{
}
#endif//_MAC

}
//namespace xPlatform

#endif//USE_GL
