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

#ifndef	__DEVICE_H__
#define	__DEVICE_H__

#pragma once

//*****************************************************************************
//	eDevice
//-----------------------------------------------------------------------------
class eDevice
{
public:
	eDevice() {}
	virtual ~eDevice() {}
	virtual void Init() {}
	virtual void Reset() {}
	virtual void FrameStart() {}
	virtual void FrameUpdate() {}
	virtual void FrameEnd(dword tacts) {}

	enum eIoNeed { ION_READ = 0x01, ION_WRITE = 0x02 };
	virtual bool IoRead(word port) const { return false; }
	virtual bool IoWrite(word port) const { return false; }
	virtual void IoRead(word port, byte* v, int tact) {}
	virtual void IoWrite(word port, byte v, int tact) {}
	virtual dword IoNeed() const { return 0; }
};

enum eDeviceId { D_ROM, D_RAM, D_ULA, D_KEYBOARD, D_KEMPSTON_JOY, D_KEMPSTON_MOUSE, D_BEEPER, D_AY, D_WD1793, D_TAPE, D_COUNT };

//*****************************************************************************
//	eDevices
//-----------------------------------------------------------------------------
class eDevices
{
public:
	eDevices();
	~eDevices();

	void Init();
	void Reset();

	template<class T> void Add(T* d) { _Add(T::Id(), d); }
	template<class T> T* Get() const { return (T*)_Get(T::Id()); }

	byte IoRead(word port, int tact)
	{
		byte v = 0xff;
		eDevice** dl = io_read_cache[io_read_map[port]];
		while(*dl)
			(*dl++)->IoRead(port, &v, tact);
		return v;
	}
	void IoWrite(word port, byte v, int tact)
	{
		eDevice** dl = io_write_cache[io_write_map[port]];
		while(*dl)
			(*dl++)->IoWrite(port, v, tact);
	}

	void FrameStart();
	void FrameUpdate();
	void FrameEnd(dword tacts);

protected:
	void _Add(eDeviceId id, eDevice* d);
	eDevice* _Get(eDeviceId id) const { return items[id]; }
	eDevice* items[D_COUNT];
	eDevice* items_io_read[D_COUNT + 1];
	eDevice* items_io_write[D_COUNT + 1];
	byte io_read_map[0x10000];
	byte io_write_map[0x10000];
	eDevice* io_read_cache[0x100][9];
	eDevice* io_write_cache[0x100][9];
};

#endif//__DEVICE_H__
