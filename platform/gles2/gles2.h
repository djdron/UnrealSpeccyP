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

#ifndef __GLES2_H__
#define __GLES2_H__

#pragma once

#ifdef USE_GLES2

#include "../../tools/point.h"

namespace xPlatform
{

class eGLES2
{
public:
	static eGLES2* Create();
	virtual ~eGLES2() {}
	virtual void Draw(const ePoint& pos, const ePoint& size) = 0;
};

}
//namespace xPlatform

#endif//USE_GLES2

#endif//__GLES2_H__
