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
#include "../io.h"

#include <io.h>
#include <windows.h>

namespace xIo
{

struct eFileSelectI
{
public:
	virtual ~eFileSelectI() {}
	virtual bool Valid() const = 0;
	virtual void Next() = 0;
	virtual const char* Name() const = 0;
	virtual bool IsDir() const = 0;
	virtual bool IsFile() const = 0;
};

class eWinFileSelectI : public eFileSelectI
{
public:
	eWinFileSelectI(const char* _path)
	{
		char path[MAX_PATH_LEN];
		strcpy(path, _path);
		strcat(path, "*");
		handle = _findfirst(path, &fd);
		valid = handle != -1;
	}
	virtual ~eWinFileSelectI() { if(handle != -1) _findclose(handle); }
	virtual bool Valid() const { return valid; }
	virtual void Next() { valid = _findnext(handle, &fd) == 0; }
	virtual const char* Name() const { return fd.name; }
	virtual bool IsDir() const { return fd.attrib&0x10; }
	virtual bool IsFile() const { return !IsDir(); }
	_finddata_t fd;
	intptr_t handle;
	bool valid;
};

class eWinDriveSelectI : public eFileSelectI
{
public:
	eWinDriveSelectI() : drives(GetLogicalDrives())
	{
		strcpy(drive, "a:");
		Update();
	}
	virtual bool Valid() const { return drives != 0; }
	void Update()
	{
		while(drives)
		{
			if(drives&1)
				break;
			drives >>= 1;
			++*drive;
		}
	}
	virtual void Next()
	{
		drives &= 0xfffe;
		Update();
	}
	virtual const char* Name() const { return drive; }
	virtual bool IsDir() const { return true; }
	virtual bool IsFile() const { return false; }
	dword drives;
	char drive[3];
};

eFileSelect::eFileSelect(const char* path)
{
	if(PathIsRoot(path))
		impl = new eWinDriveSelectI;
	else
		impl = new eWinFileSelectI(path);
}
eFileSelect::~eFileSelect() { delete impl; }
bool eFileSelect::Valid() const { return impl->Valid(); }
void eFileSelect::Next() { impl->Next(); }
const char* eFileSelect::Name() const { return impl->Name(); }
bool eFileSelect::IsDir() const { return impl->IsDir(); }
bool eFileSelect::IsFile() const { return impl->IsFile(); }

bool PathIsRoot(const char* path) {	return !strlen(path); }

}
//namespace xIo

#endif//_WINDOWS
