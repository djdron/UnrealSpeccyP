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

#ifndef __POINT_H__
#define __POINT_H__

#include "../std_types.h"

#pragma once

struct ePoint
{
	ePoint(int _x = 0, int _y = 0) : x(_x), y(_y) {}
	ePoint& operator+=(const ePoint& p) { x += p.x; y += p.y; return self; }
	ePoint& operator-=(const ePoint& p) { x -= p.x; y -= p.y; return self; }
	ePoint operator+(const ePoint& p) const { ePoint t = self; t += p; return t; }
	ePoint operator-(const ePoint& p) const { ePoint t = self; t -= p; return t; }
	ePoint& operator*=(float s) { x *= s; y *= s; return self; }
	ePoint operator*(float s) const { ePoint t = self; t *= s; return t; }
	ePoint& operator/=(float s) { x /= s; y /= s; return self; }
	ePoint operator/(float s) const { ePoint t = self; t /= s; return t; }
	int x, y;
};

#endif//__POINT_H__
