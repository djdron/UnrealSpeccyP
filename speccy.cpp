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

#include "std.h"

#include "speccy.h"
#include "devices/device.h"
#include "z80/z80.h"
#include "devices/memory.h"
#include "devices/ula.h"
#include "devices/input/keyboard.h"
#include "devices/input/kempston_joy.h"
#include "devices/input/kempston_mouse.h"
#include "devices/input/tape.h"
#include "devices/sound/beeper.h"
#include "devices/sound/ay.h"
#include "devices/fdd/wd1793.h"
#include "tools/profiler.h"

PROFILER_DECLARE(dev_e);
PROFILER_DECLARE(dev);
PROFILER_DECLARE(dev_s);
PROFILER_DECLARE(frame);

//=============================================================================
//	eSpeccy::eSpeccy
//-----------------------------------------------------------------------------
eSpeccy::eSpeccy() : cpu(NULL), memory(NULL), frame_tacts(0)
	, int_len(0), nmi_pending(0), t_states(0)
{
	// pentagon timings
	frame_tacts = 71680;
	int_len = 32;

	memory = new eMemory;
	devices.Add(new eRom(memory));
	devices.Add(new eRam(memory));
	devices.Add(new eUla(memory));
	devices.Add(new eKeyboard);
	devices.Add(new eKempstonJoy);
	devices.Add(new eKempstonMouse);
	devices.Add(new eBeeper);
	devices.Add(new eAY);
	devices.Add(new eWD1793(this, Device<eRom>()));
	devices.Add(new eTape(this));
	devices.Init();
	cpu = new xZ80::eZ80(memory, &devices, frame_tacts);

	Reset();
}
//=============================================================================
//	eSpeccy::~eSpeccy
//-----------------------------------------------------------------------------
eSpeccy::~eSpeccy()
{
	delete cpu;
	delete memory;
}
//=============================================================================
//	eSpeccy::Reset
//-----------------------------------------------------------------------------
void eSpeccy::Reset()
{
	cpu->Reset();
	devices.Reset();
}
//=============================================================================
//	eSpeccy::Update
//-----------------------------------------------------------------------------
void eSpeccy::Update(int* fetches)
{
	{
		PROFILER_SECTION(dev_s);
		devices.FrameStart();
	}
	{
		PROFILER_SECTION(frame);
		if(fetches)
			cpu->Replay(*fetches);
		else
			cpu->Update(int_len, &nmi_pending);
	}
	{
		PROFILER_SECTION(dev);
		devices.FrameUpdate();
	}
	{
		PROFILER_SECTION(dev_e);
		if(fetches)
			devices.FrameEnd(cpu->T());
		else
			devices.FrameEnd(cpu->FrameTacts() + cpu->T());
	}
	if(fetches)
		t_states += cpu->T();
	else
		t_states += cpu->FrameTacts();
}
