/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

class eFileSelectI
{
public:
	eFileSelectI(const char* _path) : valid(false)
	{
		memset(&dir_ent, 0, sizeof(dir_ent));
		dir = sceIoDopen(_path);
		if(dir >= 0)
			Next();
	}
	~eFileSelectI() { if(dir >= 0) sceIoDclose(dir); }
	bool Valid() const { return valid; }
	void Next() { valid = sceIoDread(dir, &dir_ent) > 0; }
	const char* Name() const { return dir_ent.d_name; }
	bool IsDir() const { return FIO_SO_ISDIR(dir_ent.d_stat.st_attr); }
	bool IsFile() const { return FIO_SO_ISREG(dir_ent.d_stat.st_attr); }
	SceUID dir;
	SceIoDirent dir_ent;
	bool valid;
};

eFileSelect::eFileSelect(const char* path) { impl = new eFileSelectI(path); }
eFileSelect::~eFileSelect() { delete impl; }
bool eFileSelect::Valid() const { return impl->Valid(); }
void eFileSelect::Next() { impl->Next(); }
const char* eFileSelect::Name() const { return impl->Name(); }
bool eFileSelect::IsDir() const { return impl->IsDir(); }
bool eFileSelect::IsFile() const { return impl->IsFile(); }

bool PathIsRoot(const char* path) {	return !strcmp(path, "/"); }

}
//namespace xIo

#endif//_PSP
