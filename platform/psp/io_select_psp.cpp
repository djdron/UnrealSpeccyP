/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2015 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _PSP

#include "../../std.h"
#include "../../tools/io_select.h"
#include "../io.h"
#include <pspkernel.h>

namespace xIo
{

class eFileSelectPSP : public eFileSelect
{
public:
	eFileSelectPSP(const char* _path) : valid(false)
	{
		memset(&dir_ent, 0, sizeof(dir_ent));
		dir = sceIoDopen(_path);
		if(dir >= 0)
			Next();
	}
	virtual ~eFileSelectPSP() { if(dir >= 0) sceIoDclose(dir); }
	virtual bool Valid() const { return valid; }
	virtual void Next() { valid = sceIoDread(dir, &dir_ent) > 0; }
	virtual const char* Name() const { return dir_ent.d_name; }
	virtual bool IsDir() const { return FIO_SO_ISDIR(dir_ent.d_stat.st_attr); }
	virtual bool IsFile() const { return FIO_SO_ISREG(dir_ent.d_stat.st_attr); }
private:
	SceUID dir;
	SceIoDirent dir_ent;
	bool valid;
};

eFileSelect* FileSelect(const char* path) { return new eFileSelectPSP(path); }

bool PathIsRoot(const char* path) {	return !strcmp(path, ResourcePath("")); }

bool MkDir(const char* path)
{
	int d = sceIoDopen(path);
	if(d >= 0) //directory already exists
	{
		sceIoDclose(d);
		return true;
	}
	return sceIoMkdir(path, 0777) == 0;
}

}
//namespace xIo

#endif//_PSP
