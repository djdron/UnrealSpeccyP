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

#include "../../platform.h"

#ifdef _ANDROID

#include "../../../options_common.h"

namespace xPlatform
{

void ProcessKey(char key, bool down)
{
	Handler()->OnKey(key, OpJoyKeyFlags() | (down ? KF_DOWN : 0));
}

}
//namespace xPlatform

#endif//_ANDROID
