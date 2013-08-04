/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_WXWIDGETS

#undef self
#include <wx/string.h>

class wxCmdLineParser;

namespace xPlatform
{

struct eCmdLine
{
	eCmdLine() : true_speed(V_DEFAULT), mode_48k(V_DEFAULT), full_screen(V_DEFAULT), size_percent(-1) {}

	enum eOptionValue { V_DEFAULT = -1, V_OFF = 0, V_ON = 1 };
	void Init(wxCmdLineParser& parser) const;
	void Parse(wxCmdLineParser& parser);
	void GetOptionValue(eOptionValue* v, wxCmdLineParser& parser, const char* opt) const;

	wxString file_to_open;
	wxString joystick;

	eOptionValue true_speed;
	eOptionValue mode_48k;
	eOptionValue full_screen;
	int size_percent;
};

}//namespace xPlatform

#endif//USE_WXWIDGETS
