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

#ifndef __OPTIONS_COMMON_H__
#define __OPTIONS_COMMON_H__

#include "tools/options.h"

#pragma once

namespace xPlatform
{

enum eJoystick { J_FIRST, J_KEMPSTON = J_FIRST, J_CURSOR, J_QAOP, J_SINCLAIR2, J_LAST };
enum eSound { S_FIRST, S_BEEPER = S_FIRST, S_AY, S_TAPE, S_LAST };
enum eVolume { V_FIRST, V_MUTE = V_FIRST, V_10, V_20, V_30, V_40, V_50, V_60, V_70, V_80, V_90, V_100, V_LAST };
enum eDrive { D_FIRST, D_A = D_FIRST, D_B, D_C, D_D, D_LAST };

OPTION_USING(eOptionString, op_last_file);
const char* OpLastFolder();

OPTION_USING(eOptionInt, op_drive);
OPTION_USING(eOptionInt, op_joy);
dword OpJoyKeyFlags();

OPTION_USING(eOptionInt, op_sound_source);
OPTION_USING(eOptionInt, op_volume);
OPTION_USING(eOptionBool, op_quit);

}
//namespace xPlatform

#endif//__OPTIONS_COMMON_H__
