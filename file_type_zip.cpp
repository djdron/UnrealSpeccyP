/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "platform/platform.h"

#ifdef USE_ZIP

#include <unzip.h>
#include "platform/io.h"
#include "options_common.h"
#include "tools/stream_memory.h"
#include "file_type.h"

namespace xIo
{
eFileSelect* FileSelectZIP(const char* contain_path, const char* contain_name);
}
//namespace xIo

namespace xPlatform
{

static struct eFileTypeZIP : public eFileType
{
	virtual bool Open(const void* data, size_t data_size) const;
	virtual bool Open(const char* name) const;
	virtual bool Contain(const char* name, char* contain_path, char* contain_name) const;
	virtual const char* Type() const { return "zip"; }
	virtual xIo::eFileSelect* FileSelect(const char* path) const
	{
		char contain_path[xIo::MAX_PATH_LEN];
		char contain_name[xIo::MAX_PATH_LEN];
		if(Contain(path, contain_path, contain_name))
			return xIo::FileSelectZIP(contain_path, contain_name);
		return NULL;
	}
private:
	bool Open(unzFile h, char* name = NULL) const;
	bool OpenCurrent(unzFile h, char* name = NULL) const;
} ft_zip;

static voidpf ZOpen(voidpf opaque, const void* filename, int mode)
{
	xIo::eStreamMemory* f = (xIo::eStreamMemory*)filename;
	f->Open();
	return f;
}
static uLong ZRead(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	xIo::eStreamMemory* f = (xIo::eStreamMemory*)stream;
	return f->Read(buf, size);
}
static ZPOS64_T ZTell(voidpf opaque, voidpf stream)
{
	xIo::eStreamMemory* f = (xIo::eStreamMemory*)stream;
	return f->Pos();
}
static long ZSeek(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
	xIo::eStreamMemory* f = (xIo::eStreamMemory*)stream;
	xIo::eStreamMemory::eSeekMode sm = xIo::eStreamMemory::S_CUR;
    switch(origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR: sm = xIo::eStreamMemory::S_CUR; break;
    case ZLIB_FILEFUNC_SEEK_END: sm = xIo::eStreamMemory::S_END; break;
    case ZLIB_FILEFUNC_SEEK_SET: sm = xIo::eStreamMemory::S_SET; break;
    default: return -1;
    }
    return f->Seek(offset, sm);
}
static int ZClose(voidpf opaque, voidpf stream)
{
	xIo::eStreamMemory* f = (xIo::eStreamMemory*)stream;
	return f->Close();
}

bool eFileTypeZIP::Open(const char* name) const
{
	char contain_path[xIo::MAX_PATH_LEN];
	char contain_name[xIo::MAX_PATH_LEN];
	if(Contain(name, contain_path, contain_name))
	{
		unzFile h = unzOpen64(contain_path);
		if(!h)
			return false;
		bool ok = false;
		if(unzLocateFile(h, contain_name, 0) == UNZ_OK)
		{
			ok = OpenCurrent(h);
		}
		unzClose(h);
		return ok;
	}
	char opened_name[xIo::MAX_PATH_LEN];
	bool ok = Open(unzOpen64(name), opened_name);
	if(ok)
	{
		char full_name[xIo::MAX_PATH_LEN];
		strcpy(full_name, name);
		strcat(full_name, "/");
		strcat(full_name, opened_name);
		OpLastFile(full_name);
	}
	return ok;
}

bool eFileTypeZIP::Open(const void* data, size_t data_size) const
{
	xIo::eStreamMemory mf(data, data_size);
	zlib_filefunc64_def zfuncs;
	zfuncs.zopen64_file = ZOpen;
	zfuncs.zread_file = ZRead;
	zfuncs.ztell64_file = ZTell;
	zfuncs.zseek64_file = ZSeek;
	zfuncs.zclose_file = ZClose;
	return Open(unzOpen2_64(&mf, &zfuncs));
}

bool eFileTypeZIP::Open(unzFile h, char* name) const
{
	if(!h)
		return false;
	bool ok = false;
	if(unzGoToFirstFile(h) == UNZ_OK)
	{
		for(;;)
		{
			if(OpenCurrent(h, name))
			{
				ok = true;
				break;
			}
			if(unzGoToNextFile(h) == UNZ_END_OF_LIST_OF_FILE)
				break;
		}
	}
	unzClose(h);
	return ok;
}

bool eFileTypeZIP::OpenCurrent(unzFile h, char* name) const
{
	unz_file_info fi;
	char n[xIo::MAX_PATH_LEN];
	if(unzGetCurrentFileInfo(h, &fi, n, xIo::MAX_PATH_LEN, NULL, 0, NULL, 0) != UNZ_OK)
		return false;
	const eFileType* t = eFileType::FindByName(n);
	if(!t)
		return false;
	if(unzOpenCurrentFile(h) != UNZ_OK)
		return false;

	bool ok = false;
	byte* buf = new byte[fi.uncompressed_size];
	if(unzReadCurrentFile(h, buf, fi.uncompressed_size) == int(fi.uncompressed_size))
	{
		ok = t->Open(buf, fi.uncompressed_size);
		if(ok && name)
			strcpy(name, n);
	}
	SAFE_DELETE_ARRAY(buf);
	unzCloseCurrentFile(h);
	return ok;
}

bool eFileTypeZIP::Contain(const char* name, char* contain_path, char* contain_name) const
{
	char n[xIo::MAX_PATH_LEN];
	strcpy(n, name);
	int l = strlen(n);
	for(int i = 0; i < l; ++i)
	{
		if(n[i] == '/' || n[i] == '\\')
		{
			char c = n[i];
			n[i] = 0;
			const eFileType* t = FindByName(n);
			if(t == this) // zip
			{
				FILE* f = fopen(n, "rb");
				if(f)
				{
					fclose(f);
					strcpy(contain_path, n);
					strcpy(contain_name, n + i + 1);
					return true;
				}
			}
			n[i] = c;
		}
	}
	return false;
}

}
//namespace xPlatform

#endif//USE_ZIP
