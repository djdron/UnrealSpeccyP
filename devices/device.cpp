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
void eDevices::FrameStart()
{
	for(int i = 0; i < D_COUNT; ++i)
	{
		items[i]->FrameStart();
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
//=============================================================================
//	eDevices::IoRead
//-----------------------------------------------------------------------------
byte eDevices::IoRead(word port, int tact)
{
	byte v = 0xff;
	eDevice** dl = items_io_read;
	while(*dl)
	{
		(*dl)->IoRead(port, &v, tact);
		++dl;
	}
	return v;
}
//=============================================================================
//	eDevices::IoWrite
//-----------------------------------------------------------------------------
void eDevices::IoWrite(word port, byte v, int tact)
{
	eDevice** dl = items_io_write;
	while(*dl)
	{
		(*dl)->IoWrite(port, v, tact);
		++dl;
	}
}
