/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../platform.h"

#ifdef USE_WEB

#include "../io.h"
#include "../../tools/io_select.h"
#include "io_web_source.h"

namespace xIo
{

class eFileSelectWEB : public eFileSelect
{
public:
	eFileSelectWEB() : current(-1)
	{
		path[0] = 0;
		path_cached[0] = 0;
	}
	void Init(const char* _path)
	{
		strcpy(path, _path);
		items.clear();
		for(const eWebSource* fs = eWebSource::First(); fs; fs = fs->Next())
		{
			if(strncmp(path, fs->Root().c_str(), fs->Root().length()) == 0)
			{
				bool need_cache = fs->NeedCache(path);
				if(need_cache && strcmp(path, path_cached) == 0)
				{
					items = items_cached;
				}
				else
				{
					fs->GetItems(&items, path);
					if(need_cache)
					{
						items_cached = items;
						strcpy(path_cached, path);
					}
				}
				Next();
				return;
			}
		}
		for(const eWebSource* fs = eWebSource::First(); fs; fs = fs->Next())
		{
			items.push_back(eWebSourceItem(fs->Root(), true));
		}
		Next();
	}
	virtual bool Valid() const { return current >= 0; }
	virtual void Next()
	{
		if(++current >= (int)items.size())
			current = -1;
	}
	virtual const char* Name() const { return items[current].name.c_str(); }
	virtual bool IsFile() const { return !IsDir(); }
	virtual bool IsDir() const { return items[current].is_dir; }

	std::string ItemURL(const char* name) const
	{
		int l = strlen(path);
		if(strncmp(name, path, l) != 0)
			return std::string();

		for(auto& i : items)
		{
			if(i.name == name + l)
				return i.url;
		}
		return std::string();
	}

private:
	char path[xIo::MAX_PATH_LEN];
	char path_cached[xIo::MAX_PATH_LEN];
	eWebSourceItems items;
	eWebSourceItems items_cached;
	int current;
};

static eFileSelectWEB fs_web;

eFileSelect* FileSelectWEB(const char* path)
{
	fs_web.Init(path);
	return &fs_web;
}

const char* FileSelectWEB_Open(const char* name)
{
	for(const eWebSource* fs = eWebSource::First(); fs; fs = fs->Next())
	{
		if(strncmp(name, fs->Root().c_str(), fs->Root().length()) == 0)
		{
			return fs->Open(name, fs_web.ItemURL(name));
		}
	}
	return NULL;
}

const char* OpenURL(const char* url, const char* name)
{
	std::string data = xPlatform::xWeb::GetURL(url);
	if(!data.empty())
	{
		const char* file_name = ProfilePath(name);
		char parent[MAX_PATH_LEN];
		GetPathParent(parent, file_name);
		PathCreate(parent);
		FILE* f = fopen(file_name, "wb");
		if(f)
		{
			bool ok = fwrite(data.c_str(), 1, data.size(), f) == data.size();
			fclose(f);
			if(ok)
				return file_name;
		}
		xPlatform::Handler()->OnOpenFile(file_name, data.c_str(), data.size());
	}
	return NULL;
}

}
//namespace xIo

#endif//USE_WEB
