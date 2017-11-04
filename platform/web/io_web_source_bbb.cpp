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

#include <json.hpp>
#include "io_web_source.h"

namespace xIo
{

static void ReplaceStr(std::string& s, const char* from, const char* to)
{
	size_t lfrom = strlen(from);
	size_t lto = strlen(to);
	for(size_t sp = 0; (sp = s.find(from, sp)) != std::string::npos; sp += lto)
	{
		s.replace(sp, lfrom, to);
	}
}

static class eWebSourceBBB : public eWebSource
{
public:
	eWebSourceBBB() : eWebSource("bbb", "http://bbb.retroscene.org") {}
	virtual bool NeedCache(const std::string& path) const { return !IsRootPath(path); }
	virtual void GetItems(eWebSourceItems* items, const std::string& path) const
	{
		if(IsRootPath(path))
		{
			items->push_back(eWebSourceItem("@", true));
			for(char i = 'a'; i <= 'z'; ++i)
			{
				items->push_back(eWebSourceItem(std::string(1, i), true));
			}
		}
		else
		{
			using namespace std;
			string url(path);
			url.erase(0, Root().length() + 1);
			url.erase(url.length() - 1, 1); // remove last /
			url.insert(0, RootWEB() + "/unreal_demos.php?l=");
			string data = xPlatform::xWeb::GetURL(url.c_str());
			if(data.length() && data.back() == ',') // remove last ','
				data.erase(data.length() - 1 , 1);
			ReplaceStr(data, "\\'", "'");
			ReplaceStr(data, "\t", " ");
			data = "[" + data + "]";
			using nlohmann::json;
			json j;
			try
			{
				j = json::parse(data);
			}
			catch(const invalid_argument&)
			{
			}
			for(auto e : j)
			{
				auto u = e.find("url");
				if(u == e.end() || !u->is_string())
					continue;
				string url = *u, file_name = url;
				string::size_type x = file_name.find_last_of('/');
				if(x != string::npos)
					file_name.erase(0, x + 1);
				items->push_back(eWebSourceItem(file_name, false, url));
			}
		}
	}
	virtual const char* Open(const std::string& _name, const std::string& _url) const
	{
		return OpenURL(_url.c_str(), _name.c_str());
	}
} fs_bbb;

}
//namespace xIo

#endif//USE_WEB
