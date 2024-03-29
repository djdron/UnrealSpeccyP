/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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

#include "../platform.h"
#include "../../tools/options.h"
#include "../../options_common.h"

#include "haiku_app.h"
#include "haiku_window.h"

namespace xPlatform
{

USPApplication::USPApplication(const char *signature) : BApplication(signature),
	refsReceived(NULL)
{
}

void
USPApplication::RefsReceived(BMessage* message)
{
	refsReceived = new BMessage(*message);
	for (int i = 0; i < CountWindows(); i++) {
		HaikuPlatformWindow *window = dynamic_cast <HaikuPlatformWindow*>(WindowAt(i));
		if (window) {
			window->PostMessage(refsReceived);
			break;
		}
	}
}

}
