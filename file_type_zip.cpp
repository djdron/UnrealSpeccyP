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
#include "tools/stream_memory.h"
#include "file_type.h"

namespace xPlatform
{

static struct eFileTypeZIP : public eFileType
{
	virtual bool Open(const void* data, size_t data_size);
	virtual const char* Type() { return "zip"; }
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

bool eFileTypeZIP::Open(const void* data, size_t data_size)
{
	xIo::eStreamMemory mf(data, data_size);
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

}
//namespace xPlatform

#endif//USE_ZIP
