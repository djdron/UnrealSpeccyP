#include "../../std.h"
#include "../platform.h"

#ifdef USE_WXWIDGETS

#include <wx/wx.h>
#include <wx/glcanvas.h>

namespace xPlatform
{

void InitSound();
void DoneSound();
void OnLoopSound();

void DrawGL(int w, int h, void* _data);

class GLCanvas : public wxGLCanvas
{
	typedef wxGLCanvas eInherited;
public:
	GLCanvas(wxWindow* parent) : eInherited(parent, wxID_ANY, canvas_attr), mouse_x(0), mouse_y(0)
	{
		context = new wxGLContext(this);
	}
	virtual void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC(this);
		int w, h;
		GetClientSize(&w, &h);
		SetCurrent(*context);
		DrawGL(w, h, Handler()->VideoData());
		SwapBuffers();
	}
	virtual void OnIdle(wxIdleEvent& event)
	{
		Handler()->OnLoop();
		OnLoopSound();
		Refresh(false);
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
		dword flags = KF_DOWN;
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
		Handler()->OnKey(key, 0);
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
	case '\'':
		if(flags&KF_SHIFT)
		{
			key = 'P';
			flags |= KF_ALT;
			flags &= ~KF_SHIFT;
		}
		break;
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
	}
	if(key > 255 || key < 32)
		key = 0;
}


class Frame: public wxFrame
{
public:
	Frame(const wxString& title, const wxPoint& pos)
		: wxFrame((wxFrame*)NULL, -1, title, pos), org_size(320, 240)
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
		menuDevice->Append(ID_DriveNext, _("Select next drive\tF6"));

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
		fd.SetWildcard(_(
				"Supported files|*.sna;*.trd;*.scl;*.tap;*.csw;*.tzx;"
								"*.SNA;*.TRD;*.SCL;*.TAP;*.CSW;*.TZX|"
				"All files|*.*|"
				"Snapshot files (*.sna)|*.sna;*.SNA|"
				"Disk images (*.trd;*.scl)|*.trd;*.scl;*.TRD;*.SCL|"
				"Tape files (*.tap;*.csw;*.tzx)|*.tap;*.csw;*.tzx;*.TAP;*.CSW;*.TZX"
				));
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
	enum
	{
		ID_Quit = 1, ID_OpenFile, ID_Reset, ID_Size100, ID_Size200,
		ID_TapeToggle, ID_DriveNext,
	};

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
	EVT_MENU(Frame::ID_DriveNext,Frame::OnDriveNext)
END_EVENT_TABLE()

class App: public wxApp
{
	virtual bool OnInit()
	{
		wxApp::OnInit();
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
		return wxApp::OnExit();
	}
};

}
//namespace xPlatform

IMPLEMENT_APP(xPlatform::App)

#endif//USE_WXWIDGETS
