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
#include <sys/stat.h>

namespace xIo
{

static const char* FixSlashes(const char* _path)
{
	static char path[MAX_PATH_LEN];
	strcpy(path, _path);
	for(char* b = path; *b; ++b)
	{
		if(*b == '/')
			*b = '\\';
	}
	return path;
}

class eDingooFileSelectI : public eFileSelect
{
public:
	eDingooFileSelectI(const char* _path)
	{
		char path[MAX_PATH_LEN];
		strcpy(path, FixSlashes(_path));
		strcat(path, "*");
		h = fsys_findfirst(path, -1, &fd);
	}
	~eDingooFileSelectI() { fsys_findclose(&fd); }
	virtual bool Valid() const { return h == 0; }
	virtual void Next() { h = fsys_findnext(&fd); }
	virtual const char* Name() const { return fd.name; }
	virtual bool IsDir() const { return FSYS_ISDIR(fd.attributes); }
	virtual bool IsFile() const { return FSYS_ISFILE(fd.attributes); }
	fsys_file_info_t fd;
	int h;
};

class eDingooDriveSelectI : public eFileSelect
{
public:
	eDingooDriveSelectI() : drives(0)
	{
		strcpy(drive, "a:");
	}
	virtual bool Valid() const { return drives < 2; }
	virtual void Next()
	{
		++drives;
		drive[0] = 'b';
	}
	virtual const char* Name() const { return drive; }
	virtual bool IsDir() const { return true; }
	virtual bool IsFile() const { return false; }
	char drive[3];
	dword drives;
};

eFileSelect* FileSelect(const char* path)
{
	if(PathIsRoot(path))
		return new eDingooDriveSelectI;
	else
		return new eDingooFileSelectI(path);

}

bool PathIsRoot(const char* path) {	return !strlen(path); }

bool MkDir(const char* _path)
{
	struct stat st;
	if(stat(_path, &st) == 0)
	{
		if(S_ISDIR(st.st_mode))
			return true;
	}
	return fsys_mkdir(FixSlashes(_path)) == 0;
}

}
//namespace xIo

#endif//_DINGOO

