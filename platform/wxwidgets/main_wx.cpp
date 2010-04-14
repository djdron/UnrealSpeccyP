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

namespace xPlatform
{

void InitSound();
void DoneSound();
void OnLoopSound();

void VsyncGL(bool on);
void DrawGL(int w, int h, void* _data, dword* _data_ui = NULL);

class GLCanvas : public wxGLCanvas
{
	typedef wxGLCanvas eInherited;
public:
	GLCanvas(wxWindow* parent)
	: eInherited(parent, wxID_ANY, canvas_attr)
	, key_flags(KF_CURSOR|KF_KEMPSTON)
	, mouse_x(0), mouse_y(0)
	{
		context = new wxGLContext(this);
	}
	virtual void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC(this);
		int w, h;
		GetClientSize(&w, &h);
		SetCurrent(*context);
		static bool vsync = false;
		bool s = !Handler()->FullSpeed();
		if(vsync != s)
		{
			vsync = s;
			VsyncGL(vsync);
		}
		DrawGL(w, h, Handler()->VideoData());
		SwapBuffers();
	}
	virtual void OnIdle(wxIdleEvent& event)
	{
		Handler()->OnLoop();
		OnLoopSound();
		Refresh(false);
		if(!Handler()->FullSpeed())
			wxMilliSleep(3);
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
		byte x = event.GetX() - mouse_x;
		byte y = -event.GetY() - mouse_y;
		mouse_x = event.GetX();
		mouse_y = -event.GetY();
		Handler()->OnMouse(MA_MOVE, x, y);
	}
	virtual void OnMouseKey(wxMouseEvent& event)
	{
		event.Skip();
		if(!HasCapture())
		{
			if(event.Button(wxMOUSE_BTN_LEFT) && event.ButtonDown())
			{
				SetCursor(wxCURSOR_BLANK);
				mouse_x = event.GetX();
				mouse_y = -event.GetY();
				CaptureMouse();
			}
		}
		else
			Handler()->OnMouse(MA_BUTTON, event.Button(wxMOUSE_BTN_LEFT) ? 0 : 1, event.ButtonDown());
	}
	virtual void OnKillFocus(wxFocusEvent& event)
	{
		KillMouseFocus();
	}
	void KillMouseFocus()
	{
		ReleaseMouse();
		SetCursor(wxNullCursor);
	}
	void TranslateKey(int& key, dword& flags);

	dword key_flags;

	static int canvas_attr[];
	DECLARE_EVENT_TABLE()

protected:

	wxGLContext* context;
	byte mouse_x, mouse_y;
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


class Frame: public wxFrame
{
public:
	Frame(const wxString& title, const wxPoint& pos)
		: wxFrame((wxFrame*)NULL, -1, title, pos)
		, org_size(320, 240)
	{
		wxMenu* menuFile = new wxMenu;
		menuFile->Append(ID_OpenFile, _("&Open...\tF3"));
		menuFile->Append(ID_Reset, _("&Reset...\tF12"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_Quit, _("E&xit"));

		wxMenu* menuWindow = new wxMenu;
		menuWindow->Append(ID_Size100, _("Size &100%\tCtrl+1"));
		menuWindow->Append(ID_Size200, _("Size &200%\tCtrl+2"));

		wxMenu* menuDevice = new wxMenu;
		menuDevice->Append(ID_TapeToggle, _("Start/Stop tape\tF5"));
		menuDevice->Append(ID_TapeFastToggle, _("Tape fast"));
		menuDevice->Append(ID_DriveNext, _("Select next drive\tF6"));

		wxMenu* menuJoy = new wxMenu;
		joy_menu.cursor = menuJoy->Append(ID_JoyCursor, _("Cursor"), _(""), wxITEM_CHECK);
		joy_menu.kempston = menuJoy->Append(ID_JoyKempston, _("Kempston"), _(""), wxITEM_CHECK);
		joy_menu.qaop = menuJoy->Append(ID_JoyQAOP, _("QAOP"), _(""), wxITEM_CHECK);
		joy_menu.sinclair2 = menuJoy->Append(ID_JoySinclair2, _("Sinclair 2"), _(""), wxITEM_CHECK);
		menuDevice->Append(-1, _("Joystick"), menuJoy);

		wxMenuBar* menuBar = new wxMenuBar;
		menuBar->Append(menuFile, _("File"));
		menuBar->Append(menuWindow, _("Window"));
		menuBar->Append(menuDevice, _("Device"));

		SetMenuBar(menuBar);

		CreateStatusBar();
		SetStatusText(_("Ready..."));

		SetClientSize(org_size*2);
		SetMinSize(ClientToWindowSize(org_size));

		gl_canvas = new GLCanvas(this);
		gl_canvas->SetFocus();

		UpdateJoyMenu();
	}

	void OnReset(wxCommandEvent& event)
	{
		if(Handler()->OnAction(A_RESET) == AR_RESET_OK)
			SetStatusText(_("Reset OK"));
		else
			SetStatusText(_("Reset FAILED"));
	}
	void OnQuit(wxCommandEvent& event)	{ Close(true); }
	void OnOpenFile(wxCommandEvent& event)
	{
		wxFileDialog fd(this);
		fd.SetWildcard(
				L"Supported files|*.sna;*.trd;*.scl;*.tap;*.csw;*.tzx;"
								L"*.SNA;*.TRD;*.SCL;*.TAP;*.CSW;*.TZX|"
				L"All files|*.*|"
				L"Snapshot files (*.sna)|*.sna;*.SNA|"
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
		if(joy_menu.kempston->IsChecked())
			gl_canvas->key_flags |= KF_KEMPSTON;
		if(joy_menu.cursor->IsChecked())
			gl_canvas->key_flags |= KF_CURSOR;
		if(joy_menu.qaop->IsChecked())
			gl_canvas->key_flags |= KF_QAOP;
		if(joy_menu.sinclair2->IsChecked())
			gl_canvas->key_flags |= KF_SINCLAIR2;
	}
	void UpdateJoyMenu()
	{
		joy_menu.kempston->Check(gl_canvas->key_flags&KF_KEMPSTON);
		joy_menu.cursor->Check(gl_canvas->key_flags&KF_CURSOR);
		joy_menu.qaop->Check(gl_canvas->key_flags&KF_QAOP);
		joy_menu.sinclair2->Check(gl_canvas->key_flags&KF_SINCLAIR2);
	}
	enum
	{
		ID_Quit = 1, ID_OpenFile, ID_Reset, ID_Size100, ID_Size200,
		ID_TapeToggle, ID_TapeFastToggle, ID_DriveNext,
		ID_JoyCursor, ID_JoyKempston, ID_JoyQAOP, ID_JoySinclair2,
	};
	struct eJoyMenuItems
	{
		wxMenuItem* kempston;
		wxMenuItem* cursor;
		wxMenuItem* qaop;
		wxMenuItem* sinclair2;
	};
	eJoyMenuItems joy_menu;

private:
	DECLARE_EVENT_TABLE()

	GLCanvas* gl_canvas;
	wxSize org_size;
};

BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_MENU(Frame::ID_OpenFile, Frame::OnOpenFile)
	EVT_MENU(Frame::ID_Reset,	Frame::OnReset)
	EVT_MENU(Frame::ID_Quit,	Frame::OnQuit)
	EVT_MENU(Frame::ID_Size100,	Frame::OnResize)
	EVT_MENU(Frame::ID_Size200,	Frame::OnResize)
	EVT_MENU(Frame::ID_TapeToggle,Frame::OnTapeToggle)
	EVT_MENU(Frame::ID_TapeFastToggle,Frame::OnTapeFastToggle)
	EVT_MENU(Frame::ID_DriveNext,Frame::OnDriveNext)
	EVT_MENU(Frame::ID_JoyKempston,Frame::OnJoy)
	EVT_MENU(Frame::ID_JoyCursor,Frame::OnJoy)
	EVT_MENU(Frame::ID_JoyQAOP,	Frame::OnJoy)
	EVT_MENU(Frame::ID_JoySinclair2,Frame::OnJoy)
END_EVENT_TABLE()

class App: public wxApp
{
	virtual bool OnInit()
	{
		wxApp::OnInit();
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
};

}
//namespace xPlatform

IMPLEMENT_APP(xPlatform::App)

#endif//USE_WXWIDGETS
