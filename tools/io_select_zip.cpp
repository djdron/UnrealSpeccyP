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

#include "../platform/platform.h"

#ifdef USE_ZIP

#include "../platform/io.h"
#include "io_select.h"
#include <unzip.h>

namespace xIo
{

class eFileSelectZIP : public eFileSelect
{
public:
	eFileSelectZIP(const char* _contain_path, const char* _contain_name) : fi_current(NULL), trim_path_len(0), dir_current(false)
	{
		memset(folders, 0, sizeof(folders));
		strcpy(path, _contain_name);
		int l = strlen(path);
		if(l > 0)
		{
			if(path[l - 1] == '\\' || path[l - 1] == '/')
			{
				path[l - 1] = 0; // cut last separator from path
				trim_path_len = l;
			}
			else
				trim_path_len = l + 1;
		}
		zip = unzOpen64(_contain_path);
		if(zip)
			Next();
	}
	virtual ~eFileSelectZIP()
	{
		if(zip)
			unzClose(zip);
		for(int i = 0; i < MAX_FOLDERS; ++i)
		{
			if(!folders[i])
				break;
			free(folders[i]);
		}
	}
	virtual bool Valid() const { return zip && fi_current; }
	virtual void Next()
	{
		for(;;)
		{
			if((fi_current ? unzGoToNextFile(zip) : unzGoToFirstFile(zip)) != UNZ_OK)
			{
				fi_current = NULL;
				return;
			}
			if(unzGetCurrentFileInfo(zip, &fi, name_current, xIo::MAX_PATH_LEN, NULL, 0, NULL, 0) != UNZ_OK)
			{
				fi_current = NULL;
				return;
			}
			fi_current = &fi;

			if(trim_path_len && strncmp(name_current, path, trim_path_len - 1) != 0)
			{
				Next();
				return;
			}
			char* folder = strchr(name_current + trim_path_len, '/');
			if(folder)
			{
				*folder = 0;
				if(AddFolder(name_current + trim_path_len))
				{
					dir_current = true;
					TrimName();
					return;
				}
				else
				{
					Next();
					return;
				}
			}
			else
			{
				dir_current = false;
				TrimName();
				return;
			}
		}
	}
	virtual const char* Name() const { return name_current; }
	virtual bool IsFile() const { return !dir_current; }
	virtual bool IsDir() const { return dir_current; }
private:
	void TrimName()
	{
		if(trim_path_len)
		{
			char trim_name[xIo::MAX_PATH_LEN];
			strcpy(trim_name, name_current + trim_path_len);
			strcpy(name_current, trim_name);
		}
	}
	enum { MAX_FOLDERS = 512 };
	bool AddFolder(const char* name)
	{
		int i = 0;
		for(; i < MAX_FOLDERS; ++i)
		{
			if(!folders[i])
				break;
			if(strcmp(folders[i], name) == 0)
				return false;
		}
		if(i < MAX_FOLDERS)
		{
			folders[i] = strdup(name);
			return true;
		}
		return false;
	}
	unzFile zip;
	unz_file_info fi;
	unz_file_info* fi_current;
	char path[xIo::MAX_PATH_LEN];
	char name_current[xIo::MAX_PATH_LEN];
	int trim_path_len;
	bool dir_current;
	char* folders[MAX_FOLDERS];
};

eFileSelect* FileSelectZIP(const char* contain_path, const char* contain_name) { return new eFileSelectZIP(contain_path, contain_name); }

}
//namespace xIo

#endif//USE_ZIP
