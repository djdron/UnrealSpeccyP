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

#include "../std.h"
#include "io.h"

namespace xIo
{

static char resource_path[MAX_PATH_LEN] = { 0 };
//=============================================================================
//	SetResourcePath
//-----------------------------------------------------------------------------
void SetResourcePath(const char* _path)
{
	strcpy(resource_path, _path);
}
static char buf[MAX_PATH_LEN];
//=============================================================================
//	ResourcePath
//-----------------------------------------------------------------------------
const char* ResourcePath(const char* _path)
{
	strcpy(buf, resource_path);
	strcat(buf, _path);
	return buf;
}

static char profile_path[MAX_PATH_LEN] = { 0 };
//=============================================================================
//	SetProfilePath
//-----------------------------------------------------------------------------
void SetProfilePath(const char* _path)
{
	strcpy(profile_path, _path);
}
//=============================================================================
//	ProfilePath
//-----------------------------------------------------------------------------
const char* ProfilePath(const char* _path)
{
	strcpy(buf, profile_path);
	strcat(buf, _path);
	return buf;
}

}
//namespace xIo
