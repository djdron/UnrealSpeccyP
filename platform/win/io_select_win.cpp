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

#ifdef _WINAPI

#include "../../std.h"
#include "../../tools/io_select.h"
#include "../io.h"

#include <io.h>
#include <windows.h>

namespace xIo
{

class eWinFileSelect : public eFileSelect
{
public:
	eWinFileSelect(const char* _path)
	{
		char path[MAX_PATH_LEN];
		strcpy(path, _path);
		strcat(path, "*");
		handle = _findfirst(path, &fd);
		valid = handle != -1;
	}
	virtual ~eWinFileSelect() { if(handle != -1) _findclose(handle); }
	virtual bool Valid() const { return valid; }
	virtual void Next() { valid = _findnext(handle, &fd) == 0; }
	virtual const char* Name() const { return fd.name; }
	virtual bool IsDir() const { return fd.attrib&0x10; }
	virtual bool IsFile() const { return !IsDir(); }
	_finddata_t fd;
	intptr_t handle;
	bool valid;
};

class eWinDriveSelect : public eFileSelect
{
public:
	eWinDriveSelect() : drives(GetLogicalDrives())
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
		drives &= ~1;
		Update();
	}
	virtual const char* Name() const { return drive; }
	virtual bool IsDir() const { return true; }
	virtual bool IsFile() const { return false; }
	dword drives;
	char drive[3];
};

eFileSelect* FileSelect(const char* path)
{
	if(PathIsRoot(path))
		return new eWinDriveSelect;
	else
		return new eWinFileSelect(path);
}

bool PathIsRoot(const char* path) {	return path[0] == 0; }

bool MkDir(const char* path)
{
	size_t len = strlen(path);
	if(len == 2 && path[1] == ':') // drive letter with ":" at the end
		return true;
	BOOL rc = CreateDirectoryA(path, NULL);
	if(!rc)
	{
		dword err = GetLastError();
		if(err != ERROR_ALREADY_EXISTS)
			return false;
	}
	return true;
}

}
//namespace xIo

#endif//_WINAPI
