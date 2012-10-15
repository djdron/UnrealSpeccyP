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

#ifdef _PSP

#include <pspkernel.h>
#include <pspctrl.h>
#include "../platform.h"
#include "../../options_common.h"

namespace xPlatform
{

static void ProcessButton(dword button, const SceCtrlData& pad, const SceCtrlData& pad_prev, byte key)
{
	bool b0 = (pad_prev.Buttons&button) != 0;
	bool b1 = (pad.Buttons&button) != 0;
	if(b1 == b0)
		return;

	if(b1) // pressed
	{
		Handler()->OnKey(key, KF_DOWN|OpJoyKeyFlags());
	}
	else // released
	{
		Handler()->OnKey(key, OpJoyKeyFlags());
	}
}

static SceCtrlData pad_prev;

void UpdateKeys()
{
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

	SceCtrlData pad;
	if(!sceCtrlPeekBufferPositive(&pad, 1))
		return;

	ProcessButton(PSP_CTRL_SELECT,	pad, pad_prev, 'm');
	ProcessButton(PSP_CTRL_START,	pad, pad_prev, 'k');
	ProcessButton(PSP_CTRL_LTRIGGER,pad, pad_prev, 'm');
	ProcessButton(PSP_CTRL_RTRIGGER,pad, pad_prev, 'k');

	ProcessButton(PSP_CTRL_UP,		pad, pad_prev, 'u');
	ProcessButton(PSP_CTRL_DOWN,	pad, pad_prev, 'd');
	ProcessButton(PSP_CTRL_LEFT,	pad, pad_prev, 'l');
	ProcessButton(PSP_CTRL_RIGHT,	pad, pad_prev, 'r');

#ifdef USE_PROFILER
	ProcessButton(PSP_CTRL_TRIANGLE,pad, pad_prev, 'p');
#else//USE_PROFILER
	ProcessButton(PSP_CTRL_TRIANGLE,pad, pad_prev, 'M');
#endif//USE_PROFILER
	ProcessButton(PSP_CTRL_CIRCLE ,	pad, pad_prev, 'e');
	ProcessButton(PSP_CTRL_CROSS ,	pad, pad_prev, 'f');
	ProcessButton(PSP_CTRL_SQUARE ,	pad, pad_prev, ' ');

	pad_prev = pad;
}

}
//namespace xPlatform

#endif//_PSP
