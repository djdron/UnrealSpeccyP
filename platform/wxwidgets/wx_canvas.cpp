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

class GLCanvas : public wxGLCanvas
{
	typedef wxGLCanvas eInherited;
public:
	GLCanvas(wxWindow* parent)
		: eInherited(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, _("GLCanvas"), canvas_attr)
		, mouse_pos(0, 0)
	{
	}
	virtual void OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
		Paint(dc);
	}
	void Paint(wxDC& dc)
	{
		int w, h;
		GetClientSize(&w, &h);
		SetCurrent();
		DrawGL(w, h);
		SwapBuffers();
	}
	virtual void OnIdle(wxIdleEvent& event)
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
	virtual void OnEraseBackground(wxEraseEvent& event) {}
	virtual void OnKeydown(wxKeyEvent& event)
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
	virtual void OnKeyup(wxKeyEvent& event)
	{
		int key = event.GetKeyCode();
//		printf("ku:%c\n", key);
		dword flags = 0;
		if(event.AltDown())		flags |= KF_ALT;
		if(event.ShiftDown())	flags |= KF_SHIFT;
		TranslateKey(key, flags);
		Handler()->OnKey(key, OpJoyKeyFlags());
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
				wxCommandEvent ev(evtMouseCapture, true);
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
			wxCommandEvent ev(evtMouseCapture, false);
			wxPostEvent(this, ev);
		}
	}
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

wxWindow* CreateGLCanvas(wxWindow* parent)
{
	return new GLCanvas(parent);
}

}
//namespace xPlatform

#endif//USE_WXWIDGETS
