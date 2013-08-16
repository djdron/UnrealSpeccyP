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

#include "../../options_common.h"

#undef self

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "wx_joystick.h"

namespace xPlatform
{

void OnLoopSound();
void TranslateKey(int& key, dword& flags);

void VsyncGL(bool on);
void DrawGL(int w, int h);

wxWindow* CreateMouseCapture(wxWindow* parent);

extern const wxEventType evtMouseCapture = wxNewEventType();
extern const wxEventType evtSetStatusText = wxNewEventType();
extern const wxEventType evtExitFullScreen = wxNewEventType();

//=============================================================================
//	GLCanvas
//-----------------------------------------------------------------------------
class GLCanvas : public wxGLCanvas
{
	typedef wxGLCanvas eInherited;
public:
	GLCanvas(wxWindow* parent);
	virtual ~GLCanvas();

private:
	void OnPaint(wxPaintEvent& event);
	void Paint(wxDC& dc);
	void OnIdle(wxIdleEvent& event);
	void OnEraseBackground(wxEraseEvent& event) {}
	void OnKeydown(wxKeyEvent& event);
	void OnKeyup(wxKeyEvent& event);
	void OnKillFocus(wxFocusEvent& event);
	void OnMouseKey(wxMouseEvent& event);
	void OnMouseCapture(wxCommandEvent& event);
	void OnJoystickEvent(wxJoystickEvent& event);

private:
	static int canvas_attr[];
	DECLARE_EVENT_TABLE()
	
	wxWindow* mouse_capture;
	eWxJoystick* joysticks[2];
};
int GLCanvas::canvas_attr[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };

//=============================================================================
//	EVENT_TABLE
//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
	EVT_PAINT(GLCanvas::OnPaint)
	EVT_ERASE_BACKGROUND(GLCanvas::OnEraseBackground)
	EVT_IDLE(GLCanvas::OnIdle)
	EVT_KEY_DOWN(GLCanvas::OnKeydown)
	EVT_KEY_UP(GLCanvas::OnKeyup)
	EVT_LEFT_DOWN(GLCanvas::OnMouseKey)
	EVT_KILL_FOCUS(GLCanvas::OnKillFocus)
	EVT_COMMAND(wxID_ANY, evtMouseCapture, GLCanvas::OnMouseCapture)
	EVT_JOYSTICK_EVENTS(GLCanvas::OnJoystickEvent)
END_EVENT_TABLE()

//=============================================================================
//	GLCanvas::GLCanvas
//-----------------------------------------------------------------------------
GLCanvas::GLCanvas(wxWindow* parent)
	: eInherited(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _("GLCanvas"), canvas_attr)
	, mouse_capture(NULL)
{
	joysticks[0] = new eWxJoystick(this, wxJOYSTICK1);
	joysticks[1] = new eWxJoystick(this, wxJOYSTICK2);
}
//=============================================================================
//	GLCanvas::~GLCanvas
//-----------------------------------------------------------------------------
GLCanvas::~GLCanvas()
{
	delete joysticks[0];
	delete joysticks[1];
}
//=============================================================================
//	GLCanvas::OnPaint
//-----------------------------------------------------------------------------
void GLCanvas::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	Paint(dc);
}
//=============================================================================
//	GLCanvas::Paint
//-----------------------------------------------------------------------------
void GLCanvas::Paint(wxDC& dc)
{
	int w, h;
	GetClientSize(&w, &h);
	SetCurrent();
	DrawGL(w, h);
	SwapBuffers();
}
//=============================================================================
//	GLCanvas::OnIdle
//-----------------------------------------------------------------------------
void GLCanvas::OnIdle(wxIdleEvent& event)
{
	if(OpQuit())
	{
		GetParent()->Close(true);
		return;
	}
	const char* err = Handler()->OnLoop();
	if(err)
	{
		wxCommandEvent ev(evtSetStatusText);
		ev.SetString(wxConvertMB2WX(err));
		ProcessEvent(ev);
	}
	OnLoopSound();
	{
		wxClientDC dc(this);
		static bool vsync = false;
		bool s = !Handler()->FullSpeed();
		if(vsync != s)
		{
			vsync = s;
			VsyncGL(vsync);
		}
		Paint(dc);
	}
	if(!Handler()->FullSpeed())
		wxMilliSleep(3);
	event.RequestMore();
}
//=============================================================================
//	GLCanvas::OnKeydown
//-----------------------------------------------------------------------------
void GLCanvas::OnKeydown(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
	if(key == WXK_ESCAPE)
	{
		if(mouse_capture)
		{
			mouse_capture->Close();
		}
		else
		{
			wxCommandEvent ev(evtExitFullScreen);
			wxPostEvent(this, ev);
		}
		return;
	}
//		printf("kd:%c\n", key);
	dword flags = KF_DOWN|OpJoyKeyFlags();
	if(event.AltDown())		flags |= KF_ALT;
	if(event.ShiftDown())	flags |= KF_SHIFT;
	TranslateKey(key, flags);
	Handler()->OnKey(key, flags);
}
//=============================================================================
//	GLCanvas::OnKeyup
//-----------------------------------------------------------------------------
void GLCanvas::OnKeyup(wxKeyEvent& event)
{
	int key = event.GetKeyCode();
//		printf("ku:%c\n", key);
	dword flags = 0;
	if(event.AltDown())		flags |= KF_ALT;
	if(event.ShiftDown())	flags |= KF_SHIFT;
	TranslateKey(key, flags);
	Handler()->OnKey(key, OpJoyKeyFlags());
}
//=============================================================================
//	GLCanvas::OnKeyup
//-----------------------------------------------------------------------------
void GLCanvas::OnMouseKey(wxMouseEvent& event)
{
	event.Skip();
	if(!mouse_capture)
		mouse_capture = CreateMouseCapture(this);
}
//=============================================================================
//	GLCanvas::OnKillFocus
//-----------------------------------------------------------------------------
void GLCanvas::OnKillFocus(wxFocusEvent& event)
{
	SAFE_CALL(mouse_capture)->Close();
}
//=============================================================================
//	GLCanvas::OnMouseCapture
//-----------------------------------------------------------------------------
void GLCanvas::OnMouseCapture(wxCommandEvent& event)
{
	event.Skip();
	if(!event.GetId())
		mouse_capture = NULL;
}
//=============================================================================
//	GLCanvas::OnJoystickEvent
//-----------------------------------------------------------------------------
void GLCanvas::OnJoystickEvent(wxJoystickEvent& event)
{
	switch(event.GetJoystick())
	{
	case wxJOYSTICK1: joysticks[0]->OnEvent(event); break;
	case wxJOYSTICK2: joysticks[1]->OnEvent(event); break;
	}
}

//=============================================================================
//	CreateGLCanvas
//-----------------------------------------------------------------------------
wxWindow* CreateGLCanvas(wxWindow* parent)
{
	return new GLCanvas(parent);
}

}
//namespace xPlatform

#endif//USE_WXWIDGETS
