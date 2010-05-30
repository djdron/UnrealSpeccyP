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

#include "../../std.h"
#include "../platform.h"

#ifdef USE_WXWIDGETS

#undef self

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/aboutdlg.h>
#include <wx/dnd.h>
#include <wx/cmdline.h>

namespace xPlatform
{

void InitSound();
void DoneSound();
void OnLoopSound();

void VsyncGL(bool on);
void DrawGL(int w, int h);

const wxEventType evtMouseCapture = wxNewEventType();
enum wxEventMouseCaptureId { evID_MOUSE_CAPTURED = 1, evID_MOUSE_RELEASED };

class GLCanvas : public wxGLCanvas
{
	typedef wxGLCanvas eInherited;
public:
	GLCanvas(wxWindow* parent)
		: eInherited(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _("GLCanvas"), canvas_attr)
		, key_flags(KF_CURSOR|KF_KEMPSTON)
		, mouse_pos(0, 0)
	{
	}
	virtual void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC(this);
		int w, h;
		GetClientSize(&w, &h);
		SetCurrent();
		static bool vsync = false;
		bool s = !Handler()->FullSpeed();
		if(vsync != s)
		{
			vsync = s;
			VsyncGL(vsync);
		}
		DrawGL(w, h);
		SwapBuffers();
	}
	virtual void OnIdle(wxIdleEvent& event)
	{
		Handler()->OnLoop();
		OnLoopSound();
		Refresh(false);
		if(!Handler()->FullSpeed())
			wxMilliSleep(3);
		event.RequestMore();
	}
	virtual void OnEraseBackground(wxEraseEvent& event) {}
	virtual void OnKeydown(wxKeyEvent& event)
	{
		int key = event.GetKeyCode();
		if(HasCapture() && key == WXK_ESCAPE)
		{
			KillMouseFocus();
			return;
		}
//		printf("kd:%c\n", key);
		dword flags = KF_DOWN|key_flags;
		if(event.AltDown())			flags |= KF_ALT;
		if(event.ShiftDown())		flags |= KF_SHIFT;
		TranslateKey(key, flags);
		Handler()->OnKey(key, flags);
	}
	virtual void OnKeyup(wxKeyEvent& event)
	{
		int key = event.GetKeyCode();
//		printf("ku:%c\n", key);
		dword flags = 0;
		if(event.AltDown())			flags |= KF_ALT;
		if(event.ShiftDown())		flags |= KF_SHIFT;
		TranslateKey(key, flags);
		Handler()->OnKey(key, key_flags);
	}
	virtual void OnMouseMove(wxMouseEvent& event)
	{
		event.Skip();
		if(!HasCapture())
			return;
		wxPoint p = event.GetPosition();
		wxPoint d = p - mouse_pos;
		if(d.x || d.y)
		{
			WarpPointer(mouse_pos.x, mouse_pos.y);
		}
		Handler()->OnMouse(MA_MOVE, d.x, -d.y);
	}
	virtual void OnMouseKey(wxMouseEvent& event)
	{
		event.Skip();
#ifndef _MAC
		if(!HasCapture())
		{
			if(event.Button(wxMOUSE_BTN_LEFT) && event.ButtonDown())
			{
				wxImage image_blank(1, 1);
				image_blank.SetMask();
				image_blank.SetMaskColour(0, 0, 0);
				SetCursor(image_blank);
				mouse_pos = event.GetPosition();
				CaptureMouse();
				wxCommandEvent ev(evtMouseCapture, evID_MOUSE_CAPTURED);
				wxPostEvent(this, ev);
			}
		}
		else
			Handler()->OnMouse(MA_BUTTON, event.Button(wxMOUSE_BTN_LEFT) ? 0 : 1, event.ButtonDown());
#endif//_MAC
	}
	virtual void OnKillFocus(wxFocusEvent& event)
	{
		KillMouseFocus();
	}
	void KillMouseFocus()
	{
		if(HasCapture())
		{
			ReleaseMouse();
			SetCursor(wxNullCursor);
			wxCommandEvent ev(evtMouseCapture, evID_MOUSE_RELEASED);
			wxPostEvent(this, ev);
		}
	}
	void TranslateKey(int& key, dword& flags);

	dword key_flags;

	static int canvas_attr[];
	DECLARE_EVENT_TABLE()

protected:
	wxPoint mouse_pos;
};
int GLCanvas::canvas_attr[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
	EVT_PAINT(GLCanvas::OnPaint)
	EVT_ERASE_BACKGROUND(GLCanvas::OnEraseBackground)
	EVT_IDLE(GLCanvas::OnIdle)
	EVT_KEY_DOWN(GLCanvas::OnKeydown)
	EVT_KEY_UP(GLCanvas::OnKeyup)
	EVT_MOTION(GLCanvas::OnMouseMove)
	EVT_LEFT_DOWN(GLCanvas::OnMouseKey)
	EVT_LEFT_UP(GLCanvas::OnMouseKey)
	EVT_RIGHT_DOWN(GLCanvas::OnMouseKey)
	EVT_RIGHT_UP(GLCanvas::OnMouseKey)
	EVT_KILL_FOCUS(GLCanvas::OnKillFocus)
END_EVENT_TABLE()

void GLCanvas::TranslateKey(int& key, dword& flags)
{
	switch(key)
	{
	case WXK_SHIFT:		key = 'c';	break;
	case WXK_ALT:		key = 's';	break;
	case WXK_RETURN:	key = 'e';	break;
	case WXK_TAB:
		key = '\0';
		flags |= KF_ALT;
		flags |= KF_SHIFT;
		break;
	case WXK_BACK:
		key = '0';
		flags |= KF_SHIFT;
		break;
	case WXK_LEFT:		key = 'l';	break;
	case WXK_RIGHT:		key = 'r';	break;
	case WXK_UP:		key = 'u';	break;
	case WXK_DOWN:		key = 'd';	break;
	case WXK_CONTROL:	key = 'f';	flags &= ~KF_CTRL; break;
	case '!':	key = '1';		break;
	case '@':	key = '2';		break;
	case '#':	key = '3';		break;
	case '$':	key = '4';		break;
	case '%':	key = '5';		break;
	case '^':	key = '6';		break;
	case '&':	key = '7';		break;
	case '*':	key = '8';		break;
	case '(':	key = '9';		break;
	case ')':	key = '0';		break;
	case '~':	key = '`';		break;
	case '\'':
		if(flags&KF_SHIFT)
		{
			key = 'P';
			flags &= ~KF_SHIFT;
		}
		else
			key = '7';
		flags |= KF_ALT;
		break;
	case ',':
		key = 'N';
		flags |= KF_ALT;
		break;
	case '.':
		key = 'M';
		flags |= KF_ALT;
		break;
	case ';':
		if(flags&KF_SHIFT)
		{
			key = 'Z';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'O';
		flags |= KF_ALT;
		break;
	case '/':
		if(flags&KF_SHIFT)
		{
			key = 'C';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'V';
		flags |= KF_ALT;
		break;
	case '-':
		if(flags&KF_SHIFT)
		{
			key = '0';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'J';
		flags |= KF_ALT;
		break;
	case '=':
		if(flags&KF_SHIFT)
		{
			key = 'K';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'L';
		flags |= KF_ALT;
		break;
	}
	if(key > 255 || key < 32)
		key = 0;
}

#ifndef _MAC
struct DropFilesTarget : public wxFileDropTarget
{
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
	{
		if(filenames.empty())
			return false;
		return Handler()->OnOpenFile(wxConvertWX2MB(filenames[0].c_str()));
	}
};
#endif//_MAC

struct eOptions
{
	eOptions() : true_speed(false), size_percent(200) {}
	wxString file_to_open;
	bool true_speed;
	int size_percent;
};
static eOptions options;

class Frame: public wxFrame
{
public:
	Frame(const wxString& title, const wxPoint& pos)
		: wxFrame((wxFrame*)NULL, -1, title, pos)
		, org_size(320, 240)
	{
#ifdef _WINDOWS
		SetIcon(wxICON(unreal_speccy_portable));
#endif//_WINDOWS
		wxMenu* menuFile = new wxMenu;
		menuFile->Append(wxID_OPEN, _("&Open...\tF3"));
		menuFile->Append(wxID_SAVE, _("&Save...\tF2"));
#ifdef _MAC
		menuFile->Append(wxID_ABOUT, _("About ") + title);
#else//_MAC
		SetDropTarget(new DropFilesTarget);
		menuFile->AppendSeparator();
#endif//_MAC
		menuFile->Append(wxID_EXIT, _("E&xit"));

		wxMenu* menuDevice = new wxMenu;
		menuDevice->Append(ID_TapeToggle, _("Start/Stop tape\tF5"));
		menuDevice->Append(ID_TapeFastToggle, _("Tape fast"));
		menuDevice->Append(ID_DriveNext, _("Select next drive\tF6"));
		menu_pause = menuDevice->Append(ID_PauseToggle, _("Pause\tF7"), _(""), wxITEM_CHECK);
		menu_true_speed = menuDevice->Append(ID_TrueSpeedToggle, _("True speed\tF8"), _(""), wxITEM_CHECK);
		menuDevice->Append(ID_Reset, _("&Reset...\tF12"));

		wxMenu* menuJoy = new wxMenu;
		menu_joy.cursor = menuJoy->Append(ID_JoyCursor, _("Cursor"), _(""), wxITEM_CHECK);
		menu_joy.kempston = menuJoy->Append(ID_JoyKempston, _("Kempston"), _(""), wxITEM_CHECK);
		menu_joy.qaop = menuJoy->Append(ID_JoyQAOP, _("QAOP"), _(""), wxITEM_CHECK);
		menu_joy.sinclair2 = menuJoy->Append(ID_JoySinclair2, _("Sinclair 2"), _(""), wxITEM_CHECK);
		menuDevice->Append(-1, _("Joystick"), menuJoy);

		wxMenu* menuWindow = new wxMenu;
		menuWindow->Append(ID_Size100, _("Size &100%\tCtrl+1"));
		menuWindow->Append(ID_Size200, _("Size &200%\tCtrl+2"));

		wxMenuBar* menuBar = new wxMenuBar;
		menuBar->Append(menuFile, _("File"));
		menuBar->Append(menuDevice, _("Device"));
		menuBar->Append(menuWindow, _("Window"));

#ifndef _MAC
		wxMenu* menuHelp = new wxMenu;
		menuHelp->Append(wxID_ABOUT, _("About ") + title);
		menuBar->Append(menuHelp, _("Help"));
#endif//_MAC

		SetMenuBar(menuBar);

		CreateStatusBar();
		SetStatusText(_("Ready..."));

		SetClientSize(org_size);
		SetMinSize(GetSize());
		SetClientSize(org_size*options.size_percent/100);

		gl_canvas = new GLCanvas(this);
		gl_canvas->SetFocus();

		UpdateJoyMenu();
		if(!options.file_to_open.empty())
			Handler()->OnOpenFile(wxConvertWX2MB(options.file_to_open));
		if(options.true_speed)
		{
			Handler()->OnAction(A_TRUE_SPEED_TOGGLE);
			menu_true_speed->Check(Handler()->TrueSpeed());
		}
	}

	void OnReset(wxCommandEvent& event)
	{
		if(Handler()->OnAction(A_RESET) == AR_OK)
			SetStatusText(_("Reset OK"));
		else
			SetStatusText(_("Reset FAILED"));
	}
	void OnQuit(wxCommandEvent& event)	{ Close(true); }
#ifndef _MAC
	void OnAbout(wxCommandEvent& event)
	{
		wxAboutDialogInfo info;
		info.SetName(GetTitle());
		info.SetVersion(_("0.0.9"));
		info.SetDescription(_("Portable ZX-Spectrum emulator."));
		info.SetCopyright(_("Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor."));
		info.SetWebSite(_("http://code.google.com/p/unrealspeccyp/"));
		info.SetLicense(_(
"This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
				));
		wxAboutBox(info);
	}
#endif//_MAC
	void OnOpenFile(wxCommandEvent& event)
	{
		wxFileDialog fd(this);
		fd.SetWildcard(
				L"Supported files|*.sna;*.z80;*.trd;*.scl;*.tap;*.csw;*.tzx;"
								L"*.SNA;*.Z80;*.TRD;*.SCL;*.TAP;*.CSW;*.TZX|"
				L"All files|*.*|"
				L"Snapshot files (*.sna;*.z80)|*.sna;*.z80;*.SNA;*.Z80|"
				L"Disk images (*.trd;*.scl)|*.trd;*.scl;*.TRD;*.SCL|"
				L"Tape files (*.tap;*.csw;*.tzx)|*.tap;*.csw;*.tzx;*.TAP;*.CSW;*.TZX"
			);
		if(fd.ShowModal() == wxID_OK)
		{
			if(Handler()->OnOpenFile(wxConvertWX2MB(fd.GetPath().c_str())))
				SetStatusText(_("File open OK"));
			else
				SetStatusText(_("File open FAILED"));
		}
	}
	void OnSaveFile(wxCommandEvent& event)
	{
		Handler()->VideoPaused(true);
		wxFileDialog fd(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
		fd.SetWildcard(
				L"Snapshot files (*.sna)|*.sna;*.SNA|"
				L"All files|*.*"
			);
		if(fd.ShowModal() == wxID_OK)
		{
			wxString path = fd.GetPath();
			size_t p = path.length() - 4;
			if(path.length() < 4 || (path.rfind(L".sna") != p && path.rfind(L".SNA") != p))
				path += L".sna";
			if(Handler()->OnSaveFile(wxConvertWX2MB(path.c_str())))
				SetStatusText(_("File save OK"));
			else
				SetStatusText(_("File save FAILED"));
		}
		Handler()->VideoPaused(false);
	}
	void OnResize(wxCommandEvent& event)
	{
		switch(event.GetId())
		{
		case ID_Size100: SetClientSize(org_size); break;
		case ID_Size200: SetClientSize(org_size*2); break;
		}
	}
	void OnTapeToggle(wxCommandEvent& event)
	{
		switch(Handler()->OnAction(A_TAPE_TOGGLE))
		{
		case AR_TAPE_STARTED:	SetStatusText(_("Tape started"));	break;
		case AR_TAPE_STOPPED:	SetStatusText(_("Tape stopped"));	break;
		case AR_TAPE_NOT_INSERTED:	SetStatusText(_("Tape not inserted"));	break;
		default: break;
		}
	}
	void OnTapeFastToggle(wxCommandEvent& event)
	{
		switch(Handler()->OnAction(A_TAPE_FAST_TOGGLE))
		{
		case AR_TAPE_FAST_SET:		SetStatusText(_("Fast tape speed"));		break;
		case AR_TAPE_FAST_RESET:	SetStatusText(_("Normal tape speed"));	break;
		case AR_TAPE_NOT_INSERTED:	SetStatusText(_("Tape not inserted"));	break;
		default: break;
		}
	}
	void OnDriveNext(wxCommandEvent& event)
	{
		switch(Handler()->OnAction(A_DRIVE_NEXT))
		{
		case AR_DRIVE_A:	SetStatusText(_("Selected drive A"));	break;
		case AR_DRIVE_B:	SetStatusText(_("Selected drive B"));	break;
		case AR_DRIVE_C:	SetStatusText(_("Selected drive C"));	break;
		case AR_DRIVE_D:	SetStatusText(_("Selected drive D"));	break;
		default: break;
		}
	}
	void OnJoy(wxCommandEvent& event)
	{
		gl_canvas->key_flags = 0;
		if(menu_joy.kempston->IsChecked())
			gl_canvas->key_flags |= KF_KEMPSTON;
		if(menu_joy.cursor->IsChecked())
			gl_canvas->key_flags |= KF_CURSOR;
		if(menu_joy.qaop->IsChecked())
			gl_canvas->key_flags |= KF_QAOP;
		if(menu_joy.sinclair2->IsChecked())
			gl_canvas->key_flags |= KF_SINCLAIR2;
	}
	void OnPauseToggle(wxCommandEvent& event)
	{
		if(menu_pause->IsChecked())
		{
			Handler()->VideoPaused(true);
			SetStatusText(_("Paused..."));
		}
		else
		{
			Handler()->VideoPaused(false);
			SetStatusText(_("Ready..."));
		}
	}
	void OnTrueSpeedToggle(wxCommandEvent& event)
	{
		Handler()->OnAction(A_TRUE_SPEED_TOGGLE);
		menu_true_speed->Check(Handler()->TrueSpeed());
		DoneSound();
		InitSound();
		if(Handler()->TrueSpeed())
			SetStatusText(_("True speed (50Hz mode) on"));
		else
			SetStatusText(_("True speed off"));
	}
	void OnMouseCapture(wxCommandEvent& event)
	{
		switch(event.GetId())
		{
		case evID_MOUSE_CAPTURED:	SetStatusText(_("Mouse captured, press ESC to cancel"));	break;
		case evID_MOUSE_RELEASED:	SetStatusText(_("Mouse released"));	break;
		}
	}
	void UpdateJoyMenu()
	{
		menu_joy.kempston->Check(gl_canvas->key_flags&KF_KEMPSTON);
		menu_joy.cursor->Check(gl_canvas->key_flags&KF_CURSOR);
		menu_joy.qaop->Check(gl_canvas->key_flags&KF_QAOP);
		menu_joy.sinclair2->Check(gl_canvas->key_flags&KF_SINCLAIR2);
	}
	enum
	{
		ID_Reset = 1, ID_Size100, ID_Size200,
		ID_TapeToggle, ID_TapeFastToggle, ID_DriveNext,
		ID_JoyCursor, ID_JoyKempston, ID_JoyQAOP, ID_JoySinclair2,
		ID_PauseToggle, ID_TrueSpeedToggle
	};
	struct eJoyMenuItems
	{
		wxMenuItem* kempston;
		wxMenuItem* cursor;
		wxMenuItem* qaop;
		wxMenuItem* sinclair2;
	};
	eJoyMenuItems menu_joy;
	wxMenuItem* menu_pause;
	wxMenuItem* menu_true_speed;

private:
	DECLARE_EVENT_TABLE()

	GLCanvas* gl_canvas;
	wxSize org_size;
};

BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_MENU(wxID_EXIT,				Frame::OnQuit)
#ifndef _MAC
	EVT_MENU(wxID_ABOUT,			Frame::OnAbout)
#endif//_MAC
	EVT_MENU(wxID_OPEN,				Frame::OnOpenFile)
	EVT_MENU(wxID_SAVE,				Frame::OnSaveFile)
	EVT_MENU(Frame::ID_Reset,		Frame::OnReset)
	EVT_MENU(Frame::ID_Size100,		Frame::OnResize)
	EVT_MENU(Frame::ID_Size200,		Frame::OnResize)
	EVT_MENU(Frame::ID_TapeToggle,	Frame::OnTapeToggle)
	EVT_MENU(Frame::ID_TapeFastToggle,Frame::OnTapeFastToggle)
	EVT_MENU(Frame::ID_DriveNext,	Frame::OnDriveNext)
	EVT_MENU(Frame::ID_JoyKempston,	Frame::OnJoy)
	EVT_MENU(Frame::ID_JoyCursor,	Frame::OnJoy)
	EVT_MENU(Frame::ID_JoyQAOP,		Frame::OnJoy)
	EVT_MENU(Frame::ID_JoySinclair2,Frame::OnJoy)
	EVT_MENU(Frame::ID_PauseToggle,	Frame::OnPauseToggle)
	EVT_MENU(Frame::ID_TrueSpeedToggle,	Frame::OnTrueSpeedToggle)
	EVT_COMMAND(wxID_ANY, evtMouseCapture, Frame::OnMouseCapture)
END_EVENT_TABLE()

class App: public wxApp
{
	virtual bool OnInit()
	{
		if(!wxApp::OnInit())
			return false;
		Handler()->OnInit();
		const char* c = Handler()->WindowCaption();
		Frame *frame = new Frame(wxConvertMB2WX(c), wxPoint(100, 100));
		frame->Show(true);
		SetTopWindow(frame);
		InitSound();
		return true;
	}
	virtual int OnExit()
	{
		DoneSound();
		Handler()->OnDone();
		return wxApp::OnExit();
	}
	virtual void MacOpenFile(const wxString& fileName)
	{
		Handler()->OnOpenFile(wxConvertWX2MB(fileName.c_str()));
	}
	virtual void OnInitCmdLine(wxCmdLineParser& parser)
	{
		static const wxCmdLineEntryDesc g_cmdLineDesc[] =
		{
			{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
			{ wxCMD_LINE_PARAM, NULL, NULL, wxT("input file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL  },
			{ wxCMD_LINE_SWITCH, wxT("t"), wxT("true_speed"), wxT("true speed (50Hz) mode"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL  },
			{ wxCMD_LINE_OPTION, wxT("s"), wxT("size"), wxT("window size (in percent)"), wxCMD_LINE_VAL_NUMBER, wxCMD_LINE_PARAM_OPTIONAL  },
			{ wxCMD_LINE_NONE }
		};
		parser.SetDesc(g_cmdLineDesc);
		parser.SetSwitchChars(wxT("-"));
	}
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser)
	{
		if(parser.GetParamCount())
		{
			options.file_to_open = parser.GetParam(0);
		}
		options.true_speed = parser.Found(wxT("t"));
		long size = 0;
		if(parser.Found(wxT("s"), &size))
		{
			if(size > 10 && size < 500)
				options.size_percent = size;
		}
		return true;
	}
};

}
//namespace xPlatform

IMPLEMENT_APP(xPlatform::App)

#endif//USE_WXWIDGETS
