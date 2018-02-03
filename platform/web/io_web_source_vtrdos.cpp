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

#include <regex>
#include "io_web_source.h"

namespace xIo
{

class eWebSourceVTRDOS : public eWebSource
{
public:
	eWebSourceVTRDOS(const char* _root) : eWebSource(_root, "https://vtrd.in") {}
	virtual bool NeedCache(const std::string& path) const { return !IsRootPath(path); }
	virtual const char* Open(const std::string& _name, const std::string& _url) const
	{
		if(_url.empty())
			return NULL;
		using namespace std;
		string url = RootWEB() + _url;
		return OpenURL(url.c_str(), _name.c_str());
	}
};

class eWebSourceVTRDOS_Games : public eWebSourceVTRDOS
{
public:
	eWebSourceVTRDOS_Games() : eWebSourceVTRDOS("vtrdos") {}
	virtual void GetItems(eWebSourceItems* items, const std::string& path) const
	{
		if(IsRootPath(path))
		{
			items->push_back(eWebSourceItem("full_ver", true));
			items->push_back(eWebSourceItem("demo_ver", true));
			items->push_back(eWebSourceItem("translat", true));
			items->push_back(eWebSourceItem("remix", true));
			items->push_back(eWebSourceItem("123", true));
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
			url.insert(0, RootWEB() + "/games.php?t=");
			string data = xPlatform::xWeb::GetURL(url.c_str());
			smatch m;
			regex expr("<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td><td>(.+)</td><td>(.+)</td><td>(.+)</td>");
			string::const_iterator beg = data.begin(), end = data.end();
			while(std::regex_search(beg, end, m, expr))
			{
				string file_name = m[1].str();
				if(!file_name.empty())
				{
					string::size_type x = file_name.find_last_of('/');
					if(x != string::npos)
						file_name.erase(0, x + 1);
					items->push_back(eWebSourceItem(file_name, false, m[1].str()));
				}
				beg = m[0].second;
			}
		}
	}
};

class eWebSourceVTRDOS_Press : public eWebSourceVTRDOS
{
public:
	eWebSourceVTRDOS_Press() : eWebSourceVTRDOS("press") {}
	virtual void GetItems(eWebSourceItems* items, const std::string& path) const
	{
		if(IsRootPath(path))
		{
			items->push_back(eWebSourceItem("0", true));
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
			char l = tolower(url[0]);
			url = RootWEB() + "/press.php?l=" + (l > 'n' ? "2" : "1");
			string data = xPlatform::xWeb::GetURL(url.c_str());
			regex expr("<a class=\"rpad\" href=\"(.+?)\">.+?</a>");
			string::const_iterator beg = data.begin(), end = data.end();
			smatch m;
			while(regex_search(beg, end, m, expr))
			{
				beg = m[0].second;
				string file_name = m[1].str();
				string::size_type p = file_name.find("/press/");
				if(p != 0 || file_name.size() < 8)
					continue;
				char ch0 = file_name[7];
				char ch1 = l;
				if(isdigit(ch1))
				{
					if(isalpha(ch0))
						continue;
				}
				else if(ch0 != ch1)
					continue;
				string::size_type x = file_name.rfind('/');
				if(x != string::npos)
					file_name.erase(0, x + 1);
				items->push_back(eWebSourceItem(file_name, false, m[1].str()));
			}
		}
	}
};
static eWebSourceVTRDOS_Press fs_vtrdos_press;
static eWebSourceVTRDOS_Games fs_vtrdos_games;

}
//namespace xIo

#endif//USE_WEB
