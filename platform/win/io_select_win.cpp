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

#ifdef _WINDOWS

#include "../../std.h"
#include "../../tools/io_select.h"

#include <io.h>

namespace xIo
{

class eFileSelectI
{
public:
	eFileSelectI(const char* path) { handle = _findfirst(path, &fd); valid = handle != NULL; }
	~eFileSelectI() { if(handle) _findclose(handle); }
	bool Valid() const { return valid; }
	void Next() { valid = _findnext(&fd) == 0; }
	const char* Name() const { return fd.name; }
	bool IsDir() const { return fd.attrib&0x10; }
	bool IsFile() const { return !IsDir(); }
	_finddata_t fd;
	dword handle;
	bool valid;
};

eFileSelect::eFileSelect(const char* path) { impl = new eFileSelectI(path); }
eFileSelect::~eFileSelect() { delete impl; }
bool eFileSelect::Valid() const { return impl->Valid(); }
void eFileSelect::Next() { impl->Next(); }
const char* eFileSelect::Name() const { return impl->Name(); }
bool eFileSelect::IsDir() const { return impl->IsDir(); }
bool eFileSelect::IsFile() const { return impl->IsFile(); }

}
//namespace xIo

#endif//_WINDOWS
