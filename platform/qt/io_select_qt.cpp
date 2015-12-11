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

class eFileSelectQt : public eFileSelect
{
public:
	eFileSelectQt(const char* _path)
	{
		QDir dir(_path);
		content = dir.entryInfoList();
		idx = content.size() ? 0 : -1;
		UpdateName();
	}
	virtual bool Valid() const { return idx >= 0; }
	virtual void Next()
	{
		++idx;
		if(idx >= content.size())
			idx = -1;
		UpdateName();
	}
	virtual void UpdateName()
	{
		if(idx < 0)
			name[0] = '\0';
		else
			strcpy(name, qPrintable(content[idx].fileName()));
	}

	virtual const char* Name() const { return name; }
	virtual bool IsDir() const { return content[idx].isDir(); }
	virtual bool IsFile() const { return content[idx].isFile(); }
private:
	QFileInfoList content;
	int idx;
	char name[MAX_PATH_LEN];
};

eFileSelect* FileSelect(const char* path) { return new eFileSelectQt(path); }

bool PathIsRoot(const char* path) {	return QDir(path).isRoot(); }

bool MkDir(const char* path)
{
	QDir dir(path);
	if(dir.exists())
		return true;
	return dir.mkdir(".");
}

}
//namespace xIo

#endif//USE_QT
