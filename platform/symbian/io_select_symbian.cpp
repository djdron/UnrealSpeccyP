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

#ifdef _SYMBIAN

#include "../../std.h"
#include "../../tools/io_select.h"
#include "../io.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace xIo
{

class eFileSelectI
{
public:
	virtual ~eFileSelectI() {}
	virtual bool Valid() const = 0;
	virtual void Next() = 0;
	virtual const char* Name() const = 0;
	virtual bool IsDir() const = 0;
	virtual bool IsFile() const = 0;
};

class eSymbFileSelectI : public eFileSelectI
{
public:
	eSymbFileSelectI(const char* _path) : path(_path), dir_ent(NULL)
	{
		dir = opendir(path);
		if(dir)
			Next();
	}
	virtual ~eSymbFileSelectI() { closedir(dir); }
	bool Valid() const { return dir && dir_ent; }
	void Next() { dir_ent = readdir(dir); FillStat(); }
	const char* Name() const { return dir_ent->d_name; }
	bool IsDir() const { return S_ISDIR(dir_stat.st_mode); }
	bool IsFile() const { return S_ISREG(dir_stat.st_mode); }

	void FillStat()
	{
		if(!Valid())
			return;
		memset(&dir_stat, 0, sizeof(dir_stat));
		char full_name[MAX_PATH_LEN];
		strcpy(full_name, path);
		strcat(full_name, dir_ent->d_name);
		stat(full_name, &dir_stat);
	}
	const char* path;
	DIR* dir;
	dirent* dir_ent;
	struct stat dir_stat;
	int h;
};

class eSymbDriveSelectI : public eFileSelectI
{
public:
	eSymbDriveSelectI() : drives(0)
	{
		strcpy(drive, "c:");
	}
	virtual bool Valid() const { return drives < 2; }
	virtual void Next()
	{
		++drives;
		drive[0] = 'e';
	}
	virtual const char* Name() const { return drive; }
	virtual bool IsDir() const { return true; }
	virtual bool IsFile() const { return false; }
	char drive[3];
	dword drives;
};

eFileSelect::eFileSelect(const char* path)
{
	if(PathIsRoot(path))
		impl = new eSymbDriveSelectI;
	else
		impl = new eSymbFileSelectI(path);
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

#endif//_SYMBIAN
