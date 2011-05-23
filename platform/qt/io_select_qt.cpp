/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_QT

#include "../../std.h"
#include "../../tools/io_select.h"
#include "../io.h"

#include <QDir>

namespace xIo
{

class eFileSelectI
{
public:
	eFileSelectI(const char* _path) : dir(_path)
	{
		content = dir.entryInfoList();
		idx = content.size() ? 0 : -1;
	}
	bool Valid() const { return idx >= 0; }
	void Next()
	{
		++idx;
		if(idx >= content.size())
			idx = -1;
	}
	const char* Name() const { return qPrintable(content[idx].fileName()); }
	bool IsDir() const { return content[idx].isDir(); }
	bool IsFile() const { return content[idx].isFile(); }
	QDir dir;
	QFileInfoList content;
	int idx;
};

eFileSelect::eFileSelect(const char* path) { impl = new eFileSelectI(path); }
eFileSelect::~eFileSelect() { delete impl; }
bool eFileSelect::Valid() const { return impl->Valid(); }
void eFileSelect::Next() { impl->Next(); }
const char* eFileSelect::Name() const { return impl->Name(); }
bool eFileSelect::IsDir() const { return impl->IsDir(); }
bool eFileSelect::IsFile() const { return impl->IsFile(); }

bool PathIsRoot(const char* path) {	return QDir(path).isRoot(); }

}
//namespace xIo

#endif//USE_QT
