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

#include "../platform.h"

#ifdef USE_WXWIDGETS

#include "wx_cmdline.h"
#include <wx/cmdline.h>

namespace xPlatform
{

void eCmdLine::Init(wxCmdLineParser& parser) const
{
	static const wxCmdLineEntryDesc g_cmdLineDesc[] =
	{
		{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_PARAM, NULL, NULL, wxT("input file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT("t"), wxT("true_speed"), wxT("true speed (50Hz) mode (0 or 1)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT("m"), wxT("mode_48k"), wxT("mode 48k (0 or 1)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT("f"), wxT("full_screen"), wxT("full screen mode (0 or 1)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT("s"), wxT("size"), wxT("window size (in percent)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT("j"), wxT("joystick"), wxT("use joystick (kempston, cursor, qaop, sinclair2)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_NONE }
	};
	parser.SetDesc(g_cmdLineDesc);
}

void eCmdLine::Parse(wxCmdLineParser& parser)
{
	if(parser.GetParamCount())
	{
		file_to_open = parser.GetParam(0);
	}
	GetOptionValue(&true_speed, parser, "t");
	GetOptionValue(&mode_48k, 	parser, "m");
	GetOptionValue(&full_screen, parser, "f");
	long size = 0;
	if(parser.Found(wxT("s"), &size))
	{
		if(size >= 100 && size < 500)
			size_percent = size;
	}
	wxString joy;
	if(parser.Found(wxT("j"), &joy))
	{
		joystick = joy;
	}
}

void eCmdLine::GetOptionValue(eOptionValue* v, wxCmdLineParser& parser, const char* opt) const
{
	long o = 1;
	if(parser.Found(wxConvertMB2WX(opt), &o))
	{
		*v = o ? eCmdLine::V_ON : eCmdLine::V_OFF;
	}
}

}
//namespace xPlatform

#endif//USE_WXWIDGETS
