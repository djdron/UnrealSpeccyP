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

#include "../../std.h"
#include "../../tools/io_select.h"
#include "../io.h"
#include <dingoo/fsys.h>

namespace xIo
{

class eFileSelectI
{
public:
	eFileSelectI(const char* _path)
	{
		char path[MAX_PATH_LEN];
		strcpy(path, _path);
		for(char* b = path; *b; ++b)
		{
			if(*b == '/')
				*b = '\\';
		}
		strcat(path, "*");
		h = fsys_findfirst(path, -1, &fd);
	}
	~eFileSelectI() { fsys_findclose(&fd); }
	bool Valid() const { return h == 0; }
	void Next() { h = fsys_findnext(&fd); }
	const char* Name() const { return fd.name; }
	bool IsDir() const { return fd.attributes&0x10; }
	bool IsFile() const { return !IsDir(); }
	fsys_file_info_t fd;
	int h;
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

#endif//_DINGOO
