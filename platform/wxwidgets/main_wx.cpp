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
	GLCanvas(wxWindow* parent) : eInherited(parent, wxID_ANY, canvas_attr)
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
	}
	virtual void OnEraseBackground(wxEraseEvent& event) {}

	virtual void OnKeydown(wxKeyEvent& event)
	{
		int key = event.GetKeyCode();
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
	void TranslateKey(int& key, dword& flags);

	static int canvas_attr[];
	DECLARE_EVENT_TABLE()
protected:
	wxGLContext* context;
};
int GLCanvas::canvas_attr[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
	EVT_PAINT(GLCanvas::OnPaint)
	EVT_ERASE_BACKGROUND(GLCanvas::OnEraseBackground)
	EVT_IDLE(GLCanvas::OnIdle)
	EVT_KEY_DOWN(GLCanvas::OnKeydown)
	EVT_KEY_UP(GLCanvas::OnKeyup)
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
		: wxFrame((wxFrame *)NULL, -1, title, pos), org_size(320, 240)
	{
		wxMenu* menuFile = new wxMenu;
		menuFile->Append(ID_Open, _("&Open...\tCtrl+O"));
		menuFile->Append(ID_Reset, _("&Reset...\tCtrl+R"));
		menuFile->AppendSeparator();
		menuFile->Append(ID_Quit, _("E&xit"));

		wxMenu* menuWindow = new wxMenu;
		menuWindow->Append(ID_Size100, _("Size &100%\tCtrl+1"));
		menuWindow->Append(ID_Size200, _("Size &200%\tCtrl+2"));

		wxMenuBar* menuBar = new wxMenuBar;
		menuBar->Append(menuFile, _("File"));
		menuBar->Append(menuWindow, _("Window"));

		SetMenuBar(menuBar);

		CreateStatusBar();
		SetStatusText(_("Ready..."));

		SetClientSize(org_size*2);
		SetMinSize(ClientToWindowSize(org_size));

		gl_canvas = new GLCanvas(this);
	}

	void OnReset(wxCommandEvent& event)	{ Handler()->OnReset(); }
	void OnQuit(wxCommandEvent& event)	{ Close(true); }
	void OnOpen(wxCommandEvent& event)
	{
		wxFileDialog fd(this);
		fd.ShowModal();
		const wxString& file = fd.GetPath();
		if(!file.empty())
		{
			Handler()->OnOpenFile(wxConvertWX2MB(file.c_str()));
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
	enum
	{
		ID_Quit = 1, ID_Open, ID_Reset, ID_Size100, ID_Size200,
	};

private:
	DECLARE_EVENT_TABLE()

	GLCanvas* gl_canvas;
	wxSize org_size;
};

BEGIN_EVENT_TABLE(Frame, wxFrame)
	EVT_MENU(Frame::ID_Open,	Frame::OnOpen)
	EVT_MENU(Frame::ID_Reset,	Frame::OnReset)
	EVT_MENU(Frame::ID_Quit,	Frame::OnQuit)
	EVT_MENU(Frame::ID_Size100,	Frame::OnResize)
	EVT_MENU(Frame::ID_Size200,	Frame::OnResize)
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
