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

#ifdef _DINGOO
#ifdef SIM_VERSION

#include <string.h>

extern "C"
{

/* file extension name */
int GetFileType(char* pname)
{
	if(pname)
		strcpy(pname, "TRD|SCL|SNA|Z80|TZX");
	return 0;
}

/* to get default path */
int GetDefaultPath(char* path)
{
	if(path)
		strcpy(path, "A:\\GAME");
	return 0;
}

/* module description, optional */
int GetModuleName(char* name, int code_page)
{
	if(name && (0 == code_page)) // ansi
		strcpy(name, "unreal_speccy_portable.sim");
	return 0;
}

}
//extern "C"

#endif//SIM_VERSION
#endif//_DINGOO
