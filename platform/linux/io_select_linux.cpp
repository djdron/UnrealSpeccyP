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

#if defined(_POSIX)

#include "../../std.h"
#include "../../tools/io_select.h"
#include "../io.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

namespace xIo
{

class eFileSelectPosix : public eFileSelect
{
public:
	eFileSelectPosix(const char* _path) : path(_path), dir_ent(NULL)
	{
		dir = opendir(path);
		if(dir)
			Next();
	}
	virtual ~eFileSelectPosix() { if(dir) closedir(dir); }
	virtual bool Valid() const { return dir && dir_ent; }
	virtual void Next() { dir_ent = readdir(dir); FillStat(); }
	virtual const char* Name() const { return dir_ent->d_name; }
	virtual bool IsDir() const { return S_ISDIR(dir_stat.st_mode); }
	virtual bool IsFile() const { return S_ISREG(dir_stat.st_mode); }

private:
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
};

eFileSelect* FileSelect(const char* path) { return new eFileSelectPosix(path); }

static char root_path[MAX_PATH_LEN] = "/";

void SetRootPath(const char* path) { strcpy(root_path, path); }

bool PathIsRoot(const char* path) {	return !strcmp(path, root_path); }

bool MkDir(const char* path)
{
	if(mkdir(path, 0777) != 0)
	{
		if(errno != EEXIST)
			return false;
	}
	return true;
}

}
//namespace xIo

#endif//_POSIX
