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

namespace xPlatform
{

void OnLoopSound();
void TranslateKey(int& key, dword& flags);

void VsyncGL(bool on);
void DrawGL(int w, int h);

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

private:
	virtual void OnPaint(wxPaintEvent& event);
	void Paint(wxDC& dc);
	virtual void OnIdle(wxIdleEvent& event);
	virtual void OnEraseBackground(wxEraseEvent& event) {}
	virtual void OnKeydown(wxKeyEvent& event);
	virtual void OnKeyup(wxKeyEvent& event);
	virtual void OnMouseMove(wxMouseEvent& event);
	virtual void OnMouseKey(wxMouseEvent& event);
	virtual void OnKillFocus(wxFocusEvent& event);
	void KillMouseFocus();
	void OnMouseCaptureLost(wxMouseCaptureLostEvent& event) { OnMouseCaptureLost(); }
	void OnMouseCaptureLost();
	void OnMouseCaptureChanged(wxMouseCaptureChangedEvent& event);
	static int canvas_attr[];
	DECLARE_EVENT_TABLE()

private:
	struct eMouseDelta
	{
		eMouseDelta() : x(0.0f), y(0.0f) {}
		eMouseDelta(const wxPoint& d, float sx, float sy)
		{
			x = sx*d.x;
			y = sy*d.y;
		}
		eMouseDelta& operator+=(const eMouseDelta& d) { x += d.x; y += d.y; return *this; }
		float x;
		float y;
	};
	eMouseDelta mouse_delta;
	wxPoint mouse_pos;
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
	EVT_MOTION(GLCanvas::OnMouseMove)
	EVT_LEFT_DOWN(GLCanvas::OnMouseKey)
	EVT_LEFT_UP(GLCanvas::OnMouseKey)
	EVT_RIGHT_DOWN(GLCanvas::OnMouseKey)
	EVT_RIGHT_UP(GLCanvas::OnMouseKey)
	EVT_KILL_FOCUS(GLCanvas::OnKillFocus)
	EVT_MOUSE_CAPTURE_LOST(GLCanvas::OnMouseCaptureLost)
	EVT_MOUSE_CAPTURE_CHANGED(GLCanvas::OnMouseCaptureChanged)
END_EVENT_TABLE()

//=============================================================================
//	GLCanvas::GLCanvas
//-----------------------------------------------------------------------------
GLCanvas::GLCanvas(wxWindow* parent)
	: eInherited(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _("GLCanvas"), canvas_attr)
	, mouse_pos(0, 0)
{
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
		if(HasCapture())
			KillMouseFocus();
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
//	GLCanvas::OnMouseMove
//-----------------------------------------------------------------------------
void GLCanvas::OnMouseMove(wxMouseEvent& event)
{
	event.Skip();
	if(!HasCapture())
		return;

	wxSize size = GetClientSize();
	wxPoint p = event.GetPosition();
	wxPoint d = p - mouse_pos;
	wxPoint ps = ClientToScreen(p);
	wxSize ss(wxSystemSettings::GetMetric(wxSYS_SCREEN_X), wxSystemSettings::GetMetric(wxSYS_SCREEN_Y));
	bool warp = false;
	if(ps.x < 100) 			ps.x = ss.x - 100, warp = true;
	if(ps.y < 100) 			ps.y = ss.y - 100, warp = true;
	if(ps.x > ss.x - 100)	ps.x = 100, warp = true;
	if(ps.y > ss.y - 100)	ps.y = 100, warp = true;
	if(warp)
	{
		mouse_pos = ScreenToClient(ps);
		WarpPointer(mouse_pos.x, mouse_pos.y);
	}
	else
		mouse_pos = p;

	float sx, sy;
	GetScaleWithAspectRatio43(&sx, &sy, size.x, size.y);
	float scale_x = 320.0f/size.x/sx;
	float scale_y = 240.0f/size.y/sy;
	mouse_delta += eMouseDelta(d, scale_x, scale_y);
	int dx = mouse_delta.x;
	int dy = mouse_delta.y;
	if(dx || dy)
	{
		mouse_delta.x -= dx;
		mouse_delta.y -= dy;
		Handler()->OnMouse(MA_MOVE, dx, -dy);
	}
}
//=============================================================================
//	GLCanvas::OnMouseKey
//-----------------------------------------------------------------------------
void GLCanvas::OnMouseKey(wxMouseEvent& event)
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
			wxCommandEvent ev(evtMouseCapture, true);
			wxPostEvent(this, ev);
		}
	}
	else
		Handler()->OnMouse(MA_BUTTON, event.Button(wxMOUSE_BTN_LEFT) ? 0 : 1, event.ButtonDown());
#endif//_MAC
}
//=============================================================================
//	GLCanvas::OnKillFocus
//-----------------------------------------------------------------------------
void GLCanvas::OnKillFocus(wxFocusEvent& event)
{
	KillMouseFocus();
}
//=============================================================================
//	GLCanvas::KillMouseFocus
//-----------------------------------------------------------------------------
void GLCanvas::KillMouseFocus()
{
	if(HasCapture())
	{
		ReleaseMouse();
		OnMouseCaptureLost();
	}
}
//=============================================================================
//	GLCanvas::OnMouseCaptureLost
//-----------------------------------------------------------------------------
void GLCanvas::OnMouseCaptureLost()
{
	SetCursor(wxNullCursor);
	wxCommandEvent ev(evtMouseCapture, false);
	wxPostEvent(this, ev);
//	wxLogDebug(_("OnMouseCaptureLost()"));
}
//=============================================================================
//	GLCanvas::OnMouseCaptureChanged
//-----------------------------------------------------------------------------
void GLCanvas::OnMouseCaptureChanged(wxMouseCaptureChangedEvent& event)
{
	if(event.GetCapturedWindow() != this)
	{
		OnMouseCaptureLost();
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
