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

//=============================================================================
//	eCmdLine::Init
//-----------------------------------------------------------------------------
void eCmdLine::Init(wxCmdLineParser& parser) const
{
	static const wxCmdLineEntryDesc g_cmdLineDesc[] =
	{
		{ wxCMD_LINE_SWITCH, wxT_2("h"), wxT_2("help"), wxT_2("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_PARAM, NULL, NULL, wxT_2("input file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT_2("t"), wxT_2("true_speed"), wxT_2("true speed (50Hz) mode (0 or 1)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT_2("m"), wxT_2("mode_48k"), wxT_2("mode 48k (0 or 1)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT_2("f"), wxT_2("full_screen"), wxT_2("full screen mode (0 or 1)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT_2("s"), wxT_2("size"), wxT_2("window size (in percent)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_OPTION, wxT_2("j"), wxT_2("joystick"), wxT_2("use joystick (kempston, cursor, qaop, sinclair2)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL  },
		{ wxCMD_LINE_NONE }
	};
	parser.SetDesc(g_cmdLineDesc);
}

//=============================================================================
//	eCmdLine::Parse
//-----------------------------------------------------------------------------
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
	if(parser.Found(wxT_2("s"), &size))
	{
		if(size >= 100 && size < 500)
			size_percent = size;
	}
	wxString joy;
	if(parser.Found(wxT_2("j"), &joy))
	{
		joystick = joy;
	}
}

//=============================================================================
//	eCmdLine::GetOptionValue
//-----------------------------------------------------------------------------
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
