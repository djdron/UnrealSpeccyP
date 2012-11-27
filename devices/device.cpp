/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../std.h"
#include "device.h"

//=============================================================================
//	eDevices::eDevices
//-----------------------------------------------------------------------------
eDevices::eDevices()
{
	memset(items, 0, sizeof(items));
	memset(items_io_read, 0, sizeof(items_io_read));
	memset(items_io_write, 0, sizeof(items_io_write));
}
//=============================================================================
//	eDevices::~eDevices
//-----------------------------------------------------------------------------
eDevices::~eDevices()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		SAFE_DELETE(items[i]);
	}
}
//=============================================================================
//	eDevices::Init
//-----------------------------------------------------------------------------
void eDevices::Init()
{
	int io_read_count = 0;
	for(; items_io_read[io_read_count]; ++io_read_count);
	int io_write_count = 0;
	for(; items_io_write[io_write_count]; ++io_write_count);
	assert(io_read_count <= 8 && io_write_count <= 8); //only 8 devs max supported

	for(int port = 0; port < 0x10000; ++port)
	{
		byte devs = 0;
		for(int d = 0; d < io_read_count; ++d)
		{
			if(items_io_read[d]->IoRead(port))
				devs |= 1 << d;
		}
		io_read_map[port] = devs;
		devs = 0;
		for(int d = 0; d < io_write_count; ++d)
		{
			if(items_io_write[d]->IoWrite(port))
				devs |= 1 << d;
		}
		io_write_map[port] = devs;
	}

	int size = 1 << io_read_count;
	for(int i = 0; i < size; ++i)
	{
		eDevice** dl = io_read_cache[i];
		for(int d = 0; d < io_read_count; ++d)
		{
			if((byte)i&(1 << d))
				*dl++ = items_io_read[d];
		}
		*dl = NULL;
	}
	size = 1 << io_write_count;
	for(int i = 0; i < size; ++i)
	{
		eDevice** dl = io_write_cache[i];
		for(int d = 0; d < io_write_count; ++d)
		{
			if((byte)i&(1 << d))
				*dl++ = items_io_write[d];
		}
		*dl = NULL;
	}
}
//=============================================================================
//	eDevices::Reset
//-----------------------------------------------------------------------------
void eDevices::Reset()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		items[i]->Reset();
	}
}
//=============================================================================
//	eDevices::FrameStart
//-----------------------------------------------------------------------------
void eDevices::FrameStart(dword tacts)
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		items[i]->FrameStart(tacts);
	}
}
//=============================================================================
//	eDevices::FrameUpdate
//-----------------------------------------------------------------------------
void eDevices::FrameUpdate()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		items[i]->FrameUpdate();
	}
}
//=============================================================================
//	eDevices::FrameEnd
//-----------------------------------------------------------------------------
void eDevices::FrameEnd(dword tacts)
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		items[i]->FrameEnd(tacts);
	}
}
//=============================================================================
//	eDevices::_Add
//-----------------------------------------------------------------------------
void eDevices::_Add(eDeviceId id, eDevice* d)
{
	assert(d && !items[id]);
	d->Init();
	items[id] = d;
	if(d->IoNeed()&eDevice::ION_READ)
	{
		eDevice** dl = items_io_read;
		while(*dl)
			++dl;
		*dl = d;
	}
	if(d->IoNeed()&eDevice::ION_WRITE)
	{
		eDevice** dl = items_io_write;
		while(*dl)
			++dl;
		*dl = d;
	}
}
