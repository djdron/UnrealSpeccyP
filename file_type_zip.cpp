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

#include "platform/io.h"
#include "tools/zlib/unzip.h"
#include "file_type.h"

namespace xPlatform
{

static struct eFileTypeZIP : public eFileType
{
	virtual bool Open(const void* data, size_t data_size);
	virtual const char* Type() { return "zip"; }
} ft_zip;

class eMemoryFile
{
public:
	eMemoryFile(const void* _data, size_t _data_size) : data((const byte*)_data), data_size(_data_size), ptr(NULL) {}
	void Open() { ptr = data; }
	int Close() { ptr = NULL; return 0; }
	size_t Read(void* dst, size_t size)
	{
		if(Pos() + size > data_size)
		{
			size = data_size - Pos();
		}
		memcpy(dst, ptr, size);
		ptr += size;
		return size;
	}
	size_t Pos() const { return ptr - data; }
	enum eSeekMode { S_CUR, S_END, S_SET };
	typedef long int off_t;
	int Seek(off_t offset, eSeekMode mode)
	{
		switch(mode)
		{
		case S_CUR:
			{
				const byte* new_ptr = ptr + offset;
				if(new_ptr < data || new_ptr > data + data_size)
					return -1;
				ptr = new_ptr;
				return 0;
			}
		case S_END:
			if(-offset > (off_t)data_size || offset > 0)
				return -1;
			ptr = data + data_size - offset;
			return 0;
		case S_SET:
			if(offset < 0 || offset > (off_t)data_size)
				return -1;
			ptr = data + offset;
			return 0;
		}
		return -1;
	}
protected:
	const byte* data;
	size_t data_size;
	const byte* ptr;
};
static voidpf ZOpen(voidpf opaque, const void* filename, int mode)
{
	eMemoryFile* f = (eMemoryFile*)filename;
	f->Open();
	return f;
}
static uLong ZRead(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	return f->Read(buf, size);
}
static ZPOS64_T ZTell(voidpf opaque, voidpf stream)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	return f->Pos();
}
static long ZSeek(voidpf opaque, voidpf stream, ZPOS64_T offset, int origin)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	eMemoryFile::eSeekMode sm = eMemoryFile::S_CUR;
    switch(origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR: sm = eMemoryFile::S_CUR; break;
    case ZLIB_FILEFUNC_SEEK_END: sm = eMemoryFile::S_END; break;
    case ZLIB_FILEFUNC_SEEK_SET: sm = eMemoryFile::S_SET; break;
    default: return -1;
    }
    return f->Seek(offset, sm);
}
static int ZClose(voidpf opaque, voidpf stream)
{
	eMemoryFile* f = (eMemoryFile*)stream;
	return f->Close();
}

bool eFileTypeZIP::Open(const void* data, size_t data_size)
{
	eMemoryFile mf(data, data_size);
	zlib_filefunc64_def zfuncs;
	zfuncs.zopen64_file = ZOpen;
	zfuncs.zread_file = ZRead;
	zfuncs.ztell64_file = ZTell;
	zfuncs.zseek64_file = ZSeek;
	zfuncs.zclose_file = ZClose;

	unzFile h = unzOpen2_64(&mf, &zfuncs);
	if(!h)
		return false;
	bool ok = false;
	if(unzGoToFirstFile(h) == UNZ_OK)
	{
		for(;;)
		{
			unz_file_info fi;
			char n[xIo::MAX_PATH_LEN];
			if(unzGetCurrentFileInfo(h, &fi, n, xIo::MAX_PATH_LEN, NULL, 0, NULL, 0) == UNZ_OK)
			{
				eFileType* t = eFileType::FindByName(n);
				if(t)
				{
					if(unzOpenCurrentFile(h) == UNZ_OK)
					{
						byte* buf = new byte[fi.uncompressed_size];
						if(unzReadCurrentFile(h, buf, fi.uncompressed_size) == int(fi.uncompressed_size))
						{
							ok = t->Open(buf, fi.uncompressed_size);
						}
						delete[] buf;
						unzCloseCurrentFile(h);
					}
				}
			}
			if(ok)
				break;
			if(unzGoToNextFile(h) == UNZ_END_OF_LIST_OF_FILE)
				break;
		}
	}
	unzClose(h);
	return ok;
}

#endif//USE_ZIP

}
//namespace xPlatform
