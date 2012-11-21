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
/*
                                    R  Z  X

             Input Recording Library for ZX Spectrum emulators
      =================================================================
            Library version: 0.12 - last updated: 04 August 2002
                 Created by Ramsoft ZX Spectrum demogroup

      This is free software. Permission to use it in non-commercial and
      commercial products is hereby granted at the terms of the present
      licence:

      ** WORK IN PROGRESS ** (in short: FREE FOR ALL)

      - A link to the official site of the RZX specifications and SDK
        should be provided either in the program executable or into the
        accompanying documentation.
      - The present license must be not be removed or altered.
*/
// http://ramsoft.bbk.org.omegahg.com/rzx.html
// http://www.rzxarchive.co.uk/

#include "../platform/platform.h"
#include "../platform/io.h"
#include "../tools/stream_memory.h"
#include "../options_common.h"
#include "rzx.h"

#ifdef USE_ZIP
#include "../tools/zlib/zlib.h"
#endif

class eRZX::eImpl
{
public:
	eImpl() : status(0), file(NULL), handler(NULL), framecount(0), INmax(0), INcount(0), INold(0)
		, inputbuffer(NULL)
#ifdef USE_ZIP
		,zbuf(NULL)
#endif//USE_ZIP
	{}
	~eImpl() { rzx_close(); }
	eError Open(const void* data, size_t data_size, eHandler* handler);
	eError Update(int* icount);
	eError IoRead(byte* data);
	eError CheckSync() const { return INcount == INmax ? OK : SYNCLOST; }

private:
	class eStream : public xIo::eStreamMemory
	{
	public:
		eStream(const void* _data, size_t _data_size) : xIo::eStreamMemory(_data, _data_size)
		{
			// we need copy of the data
			byte* d = new byte[data_size];
			memcpy(d, _data, data_size);
			data = d;
			Open();
		}
		~eStream()
		{
			Close();
			SAFE_DELETE_ARRAY(data);
		}
	};
	enum eBlockId
	{
		RZXBLK_CREATOR	= 0x10,
		RZXBLK_SESSION	= 0x11,
		RZXBLK_COMMENT	= 0x12,
		RZXBLK_SECURITY	= 0x20,
		RZXBLK_SNAP		= 0x30,
		RZXBLK_DATA		= 0x80,
	};
	enum eStatus
	{
		RZX_INIT	= 0x01,
		RZX_IRB		= 0x02,
		RZX_PROT	= 0x04,
		RZX_PACK	= 0x08,
	};

	enum { RZXBLKBUF = 512, RZXINPUTMAX = 32768 };
	byte status;
	eStream* file;
	eHandler* handler;

	struct RZX_BLKHDR
	{
		byte type;
		dword length;
		long start;
		byte buff[RZXBLKBUF];
	};

	RZX_BLKHDR block;
	dword framecount;

	word INmax;
	word INcount;
	word INold;
	byte* inputbuffer;

#ifdef USE_ZIP
	enum { ZBUFLEN = 16384 };
	z_stream zs;
	byte* zbuf;
	int rzx_pread(byte* buffer, int len);
	int rzx_pclose();
	int rzx_popen(long offset);
#endif//USE_ZIP
	int rzx_scan();
	void rzx_close_irb();
	int rzx_seek_irb();
	void rzx_close();
};


/* ======================================================================== */


#define LLO(x) (byte)(x&0xFF)
#define LHI(x) (byte)((x&0xFF00)>>8)
#define HLO(x) (byte)((x&0xFF0000)>>16)
#define HHI(x) (byte)((x&0xFF000000)>>24)


#ifdef USE_ZIP
int eRZX::eImpl::rzx_pread(byte *buffer, int len)
{
	zs.next_out = buffer;
	zs.avail_out = len;
	while(zs.avail_out > 0)
	{
		if(zs.avail_in == 0)
		{
			zs.avail_in = file->Read(zbuf, ZBUFLEN);
			if(zs.avail_in == 0)
				return 0;
			zs.next_in = zbuf;
		}
		inflate(&zs, Z_NO_FLUSH);
	}
	return len - zs.avail_out;
}

int eRZX::eImpl::rzx_pclose()
{
	if(zbuf != 0)
	{
		inflateEnd(&zs);
		free(zbuf);
		zbuf = 0;
	}
	return 0;
}

int eRZX::eImpl::rzx_popen(long offset)
{
	int err;
	memset(&zs, 0, sizeof(zs));
	assert(!zbuf);
	zbuf = (byte*) malloc(ZBUFLEN);
	zs.next_in = zbuf;
	err = inflateInit2(&zs,15);
	zs.avail_out = ZBUFLEN;
	if(err != Z_OK)
		return -1;
	file->Seek(offset);
	return 0;
}
#endif//USE_ZIP

int eRZX::eImpl::rzx_scan()
{
	long fpos = 10;
	do
	{
		file->Seek(fpos);
		if(file->Read(block.buff, 5) < 5)
			break;
		block.type = block.buff[0];
		block.length = block.buff[1] + 256 * block.buff[2] + 65536 * block.buff[3] + 16777216 * block.buff[4];
		/* printf("rzx_scan: block %02X len=%05i\n",block.type,block.length); */
		if(block.length == 0)
			return INVALID;
		switch(block.type)
		{
		case RZXBLK_CREATOR:
			{
				/* read the info about the emulator which created this RZX */
				file->Seek(file->Pos() + 24);
				int custom_data_size = block.length - 29;
				if(custom_data_size > 0)
					file->Seek(file->Pos() + custom_data_size);
			}
			break;
		case RZXBLK_DATA:
			file->Read(block.buff, 13);
			break;
		}
		fpos += block.length;
	} while(1);
	file->Seek(10);
	block.start = 10;
	block.length = 0;
	block.type = 0;
	return OK;
}


void eRZX::eImpl::rzx_close_irb()
{
#ifdef USE_ZIP
	rzx_pclose();
#endif
	/* suppress empty IRBs */
	if(!framecount)
	{
		file->Seek(block.start);
		status &= ~RZX_IRB;
		return;
	}
	block.start = file->Pos();
	status &= ~RZX_IRB;
}

int eRZX::eImpl::rzx_seek_irb()
{
	int done = 0;
	long fpos;
	while(!done)
	{
		if(file->Read(block.buff, 5) < 5)
			return FINISHED;
		block.type = block.buff[0];
		block.length = block.buff[1]+256*block.buff[2]+65536*block.buff[3]+16777216*block.buff[4];
		if(block.length == 0)
			return INVALID;
		switch(block.type)
		{
		case RZXBLK_SNAP:
			{
				file->Read(block.buff, 12);
				char snap_filename[xIo::MAX_PATH_LEN];
				if(!(block.buff[0] & 0x01))
				{
					/* embedded snap */
#ifdef USE_ZIP
					bool compressed = (block.buff[0] & 0x02) != 0;
					fpos = file->Pos();
					rzx_popen(fpos);
#endif
					strcpy(snap_filename, (const char*)block.buff + 4);
					size_t snap_size = block.buff[8]+256*block.buff[9]+65536*block.buff[10]+16777216*block.buff[11];
					byte* snap_data = new byte[snap_size];
					byte* snap_pos = snap_data;
					fpos = snap_size;
					while(fpos > 0)
					{
						done = (fpos > RZXBLKBUF) ? RZXBLKBUF : fpos;
#ifdef USE_ZIP
						if(compressed)
							rzx_pread(block.buff, done);
						else
#endif
							file->Read(block.buff, done);
						memcpy(snap_pos, block.buff, done);
						snap_pos += done;
						fpos -= done;
					}
#ifdef USE_ZIP
					rzx_pclose();
#endif
					done = 0;
					bool ok = handler->RZX_OnOpenSnapshot(snap_filename, snap_data, snap_size);
					SAFE_DELETE_ARRAY(snap_data);
					if(!ok)
						return UNSUPPORTED;
				}
				else
				{
					/* external snap, read descriptor */
					file->Read(&block.buff[12], block.length - 17);
					const char* snap_fullname = (const char*)block.buff + 16;
					if(!handler->RZX_OnOpenSnapshot(snap_fullname, NULL, 0))
					{
						// trying to open snapshot from the same folder where .rzx placed
						int l = strlen(snap_fullname);
						while(l >= 0 && snap_fullname[l] != '/' && snap_fullname[l] != '\\')
							--l;
						strcpy(snap_filename, xPlatform::OpLastFolder());
						strcat(snap_filename, snap_fullname + l + 1);
						if(!handler->RZX_OnOpenSnapshot(snap_filename, NULL, 0))
							return UNSUPPORTED;
					}
				}
			}
			break;
		case RZXBLK_DATA:
			/* recording block found, initialize the values */
			file->Read(block.buff, 13);
			framecount = block.buff[0]+256*block.buff[1]+65536*block.buff[2]+16777216*block.buff[3];
			status |= RZX_IRB;
			if(block.buff[9] & 0x01)
				status |= RZX_PROT;
			else
				status &= ~RZX_PROT;
			if(block.buff[9] & 0x02)
				status |= RZX_PACK;
			else
				status &= ~RZX_PACK;

			if(status & RZX_PACK)
#ifndef USE_ZIP
				return UNSUPPORTED;
#else//USE_ZIP
			{
				fpos = file->Pos();
				rzx_popen(fpos);
			}
#endif
			return OK;
		case RZXBLK_SECURITY:
			break;
		default:
			break;
		}
		/* seek the next block in the file */
		block.start += block.length;
		file->Seek(block.start);
	}
	return OK;
}


eRZX::eError eRZX::eImpl::Open(const void* _data, size_t _data_size, eHandler* _handler)
{
	assert(!file);
	file = new eStream(_data, _data_size);
	handler = _handler;
	if(!handler)
		return INVALID;
	if(!file->Size())
		return INVALID;
	inputbuffer = new byte[RZXINPUTMAX];
	memset(inputbuffer, 0, RZXINPUTMAX);
	memset(block.buff, 0, RZXBLKBUF);
	memset(&block, 0, 16);
	file->Read(block.buff, 6);
	if(memcmp(block.buff, "RZX!", 4))
	{
		rzx_close();
		return INVALID;
	}
	/* pre-scan the file to collect useful information and stats */
	if(rzx_scan() != OK)
	{
		rzx_close();
		return INVALID;
	}
	if(rzx_seek_irb() != OK)
	{
		rzx_close();
		return FINISHED;
	}
	INcount = 0;
	INold = 0xFFFF;
	return OK;
}

void eRZX::eImpl::rzx_close()
{
#ifdef USE_ZIP
	rzx_pclose();
#endif//USE_ZIP
	SAFE_DELETE(file);
	status = RZX_INIT;
	SAFE_DELETE(inputbuffer);
}


eRZX::eError eRZX::eImpl::Update(int* icount)
{
	/* check if we are at the beginning */
	if((status & RZX_IRB) && (!framecount))
		status &= ~RZX_IRB;
	/* need to seek another IRB? */
	if(!(status & RZX_IRB))
	{
		if(rzx_seek_irb() != OK)
		{
			/* no more IRBs, finished */
			rzx_close();
			return FINISHED;
		}
	}

	/* fetch the instruction and IN counters */
	INold = INmax;
#ifdef USE_ZIP
	if(status & RZX_PACK)
		rzx_pread(block.buff, 4);
	else
#endif
	file->Read(block.buff, 4);
	(*icount) = block.buff[0] + 256 * block.buff[1];
	INmax = block.buff[2] + 256 * block.buff[3];

	/* update the input array */
#ifdef USE_ZIP
	if(INmax && (INmax != 0xFFFF))
	{
		if(status & RZX_PACK)
			rzx_pread(inputbuffer, INmax);
		else
			file->Read(inputbuffer, INmax);
	}
	else
		INmax = INold;
#else
	if(INmax && (INmax != 0xFFFF))
		file->Read(inputbuffer, INmax);
	else
		INmax = INold;
#endif
	INcount = 0;
	--framecount;
	return OK;
}
eRZX::eError eRZX::eImpl::IoRead(byte* data)
{
	if(INcount >= INmax)
		return SYNCLOST;
	*data = inputbuffer[INcount++];
	return OK;
}

eRZX::eRZX() { impl = new eImpl; }
eRZX::~eRZX() { delete impl; }
eRZX::eError eRZX::Open(const void* data, size_t data_size, eHandler* handler) { return impl->Open(data, data_size, handler); }
eRZX::eError eRZX::Update(int* icount) { return impl->Update(icount); }
eRZX::eError eRZX::IoRead(byte* data) { return impl->IoRead(data); }
eRZX::eError eRZX::CheckSync() const { return impl->CheckSync(); }
