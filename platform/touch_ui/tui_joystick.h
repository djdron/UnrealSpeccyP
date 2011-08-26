/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifndef	__TUI_JOYSTICK_H__
#define	__TUI_JOYSTICK_H__

#pragma once

#ifdef _ANDROID
#define USE_TUI
#endif//_ANDROID

#ifdef USE_TUI

namespace xPlatform
{

void OnTouchJoy(float x, float y, bool down, int pointer_id);

}
//namespace xPlatform

#endif//USE_TUI

#endif//__TUI_JOYSTICK_H__
