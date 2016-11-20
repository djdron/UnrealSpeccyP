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

#ifndef __IO_WEB_SOURCE_H__
#define __IO_WEB_SOURCE_H__

#pragma once

#include <vector>
#include <string>
#include "../../tools/list.h"

namespace xIo
{

struct eWebSourceItem
{
	eWebSourceItem(const std::string& _name, bool _is_dir, const std::string& _url = std::string()) : name(_name), url(_url), is_dir(_is_dir) {}
	std::string name;
	std::string url;
	bool is_dir;
};

typedef std::vector<eWebSourceItem> eWebSourceItems;

class eWebSource : public eList<eWebSource>
{
public:
	eWebSource(const char* _root, const char* _root_web) : root(_root), root_web(_root_web) {}
	const std::string& Root() const  { return root; }
	const std::string& RootWEB() const  { return root_web; }

	bool IsRootPath(const std::string& path) const { return path == Root() + "/"; }

	virtual void GetItems(eWebSourceItems* items, const std::string& path) const = 0;
	virtual bool NeedCache(const std::string& path) const { return false; }
	virtual const char* Open(const std::string& name, const std::string& url) const = 0;
private:
	std::string root;
	std::string root_web;
};

const char* OpenURL(const char* url, const char* name);

}
//namespace xIo

namespace xPlatform
{
namespace xWeb
{
std::string GetURL(const char* path);
}
//namespace xWeb
}
//namespace xPlatform

#endif//__IO_WEB_SOURCE_H__
