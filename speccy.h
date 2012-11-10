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

#ifndef	__SPECCY_H__
#define	__SPECCY_H__

#include "devices/device.h"

#pragma once

namespace xZ80 { class eZ80; }
class eMemory;

//*****************************************************************************
//	eSpeccy
//-----------------------------------------------------------------------------
class eSpeccy
{
public:
	eSpeccy();
	virtual ~eSpeccy();

	void Reset();
	void Update(int* fetches = NULL);

	xZ80::eZ80*	CPU() const { return cpu; }
	eMemory*	Memory() const { return memory; }
	eDevices&	Devices() { return devices; }
	template<class D> D* Device() const { return devices.Get<D>(); }

	qword T() const { return t_states; }

protected:
	xZ80::eZ80* cpu;
	eMemory* memory;
	eDevices devices;

	int		frame_tacts;	// t-states per frame
	int		int_len;		// length of INT signal (for Z80)
	int		nmi_pending;
	qword	t_states;
};

#endif//__SPECCY_H__
