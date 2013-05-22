/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _CHROME_NACL

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/cpp/rect.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/fullscreen.h"
#include "ppapi/cpp/mouse_lock.h"
#include "ppapi/gles2/gl2ext_ppapi.h"

#include "nacl_gl_context.h"
#include "nacl_url.h"
#include "nacl_sound.h"
#include "../gles2/gles2.h"
#include "../platform.h"
#include "../../tools/options.h"
#include "../../options_common.h"

#include <GLES2/gl2.h>

using namespace std;

namespace xPlatform
{

void LoadResources(pp::Instance* i);

class eUSPInstance : public pp::Instance, public pp::MouseLock, public eURLLoader::eCallback
{
public:
	eUSPInstance(PP_Instance instance);
	virtual ~eUSPInstance();
	virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
	virtual void HandleMessage(const pp::Var& _m);
	virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip);
	virtual bool HandleInputEvent(const pp::InputEvent& event);
	virtual void OnURLLoadOk(const std::string& url, const char* buffer, size_t size);
	virtual void OnURLLoadFail(const std::string& url);
	virtual void MouseLockLost();

protected:
	bool 	SpecialKeyDown(const pp::KeyboardInputEvent event);
	void	TranslateKey(int& key, dword& flags);
	void	Draw();

	//callbacks
	void	Update(int32_t result = 0);
	void	DidLockMouse(int32_t result);

protected:
	eGLContext* gl_context;
	eGLES2* gles2;
	pp::Size size;
	eAudio audio;
	pp::Fullscreen full_screen;
	pp::CompletionCallbackFactory<eUSPInstance> callback;
	bool inited;
	bool mouse_locked;
};

eUSPInstance::eUSPInstance(PP_Instance instance)
	: pp::Instance(instance), pp::MouseLock(this)
	, gl_context(NULL), gles2(NULL), size(0, 0)
	, full_screen(this), callback(this)
	, inited(false), mouse_locked(false)
{
}

eUSPInstance::~eUSPInstance()
{
	if(inited)
		Handler()->OnDone();
	if(gl_context)
	{
		gl_context->MakeCurrent(this);
		delete gles2;
		delete gl_context;
	}
}

bool eUSPInstance::Init(uint32_t argc, const char* argn[], const char* argv[])
{
    RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);
	RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);
	LoadResources(this);
	audio.Init(this);
	return true;
}

void eUSPInstance::HandleMessage(const pp::Var& _m)
{
	if(!_m.is_string())
		return;
	string m = _m.AsString();
	if(m == "run")
	{
		xPlatform::Handler()->OnInit();
		inited = true;
		Update();
		audio.Play();
		PostMessage("ready");
	}
	if(!inited)
		return;
	static const string open("open:");
	static const string joystick("joystick:");
	static const string zoom("zoom:");
	static const string filtering("filtering:");
	if(m.length() > open.length() && m.substr(0, open.length()) == open)
	{
		string url = m.substr(open.length());
		new eURLLoader(this, url, this);
	}
	else if(m.length() > joystick.length() && m.substr(0, joystick.length()) == joystick)
	{
		using namespace xOptions;
		string joy = m.substr(joystick.length());
		eOption<int>* op_joy = eOption<int>::Find("joystick");
		SAFE_CALL(op_joy)->Value(joy.c_str());
	}
	else if(m.length() > zoom.length() && m.substr(0, zoom.length()) == zoom)
	{
		using namespace xOptions;
		string z = m.substr(zoom.length());
		eOption<int>* op_zoom = eOption<int>::Find("zoom");
		SAFE_CALL(op_zoom)->Value(z.c_str());
	}
	else if(m.length() > filtering.length() && m.substr(0, filtering.length()) == filtering)
	{
		using namespace xOptions;
		string f = m.substr(filtering.length());
		eOption<bool>* op_filtering = eOption<bool>::Find("filtering");
		SAFE_CALL(op_filtering)->Value(f.c_str());
	}
	else if(m == "reset")
	{
		Handler()->OnAction(A_RESET);
	}
}

void eUSPInstance::OnURLLoadOk(const std::string& url, const char* buffer, size_t size)
{
	PostMessage("open ok");
	Handler()->OnOpenFile(url.c_str(), buffer, size);
}

void eUSPInstance::OnURLLoadFail(const std::string& url)
{
	PostMessage("open failed");
}

void eUSPInstance::Update(int32_t result)
{
	pp::Module::Get()->core()->CallOnMainThread(17, callback.NewCallback(&eUSPInstance::Update));
	Handler()->OnLoop();
	audio.Update();
	Draw();
}

void eUSPInstance::DidChangeView(const pp::Rect& position, const pp::Rect& clip)
{
	if(size == position.size())
		return;
	size = position.size();
	if(!gl_context)
	{
		gl_context = new eGLContext(this);
		gl_context->MakeCurrent(this);
		gles2 = eGLES2::Create();
	}
	gl_context->Invalidate();
	gl_context->Resize(size);
	Draw();
}

void eUSPInstance::DidLockMouse(int32_t result)
{
	mouse_locked = result == PP_OK;
}

void eUSPInstance::MouseLockLost()
{
	mouse_locked = false;
}

void eUSPInstance::Draw()
{
	if(!inited || !gl_context)
		return;
	gl_context->MakeCurrent(this);
	gles2->Draw(size.width(), size.height());
	gl_context->Flush();
}

void eUSPInstance::TranslateKey(int& key, dword& flags)
{
	enum
	{
		K_SHIFT = 16, K_CTRL = 17, K_ALT = 18,
		K_TAB = 9, K_ENTER = 13, K_BACKSPACE = 8,
		K_LEFT = 37, K_UP = 38, K_RIGHT = 39, K_DOWN = 40,
		K_QUOTE = 222, K_APOSTROPHE = 192, K_BACKSLASH = 220,
		K_DOT = 190, K_COMMA = 188, K_SEMICOLON = 186, K_SLASH = 191,
		K_MINUS = 189, K_EQUAL = 187,
	};
	switch(key)
	{
	case K_SHIFT:		key = 'c';	break;
	case K_ALT:			key = 's';	break;
	case K_ENTER:		key = 'e';	break;
	case K_TAB:
		key = '\0';
		flags |= KF_ALT;
		flags |= KF_SHIFT;
		break;
	case K_BACKSPACE:
		key = '0';
		flags |= KF_SHIFT;
		break;
	case K_LEFT:		key = 'l';	break;
	case K_RIGHT:		key = 'r';	break;
	case K_UP:			key = 'u';	break;
	case K_DOWN:		key = 'd';	break;
	case K_CTRL:		key = 'f';	flags &= ~KF_CTRL; break;
	case K_APOSTROPHE:	key = 'm';	break;
	case K_BACKSLASH:	key = 'k';	break;
	case K_QUOTE:
		if(flags&KF_SHIFT)
		{
			key = 'P';
			flags &= ~KF_SHIFT;
		}
		else
			key = '7';
		flags |= KF_ALT;
		break;
	case K_COMMA:
		if(flags&KF_SHIFT)
		{
			key = 'R';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'N';
		flags |= KF_ALT;
		break;
	case K_DOT:
		if(flags&KF_SHIFT)
		{
			key = 'T';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'M';
		flags |= KF_ALT;
		break;
	case K_SEMICOLON:
		if(flags&KF_SHIFT)
		{
			key = 'Z';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'O';
		flags |= KF_ALT;
		break;
	case K_SLASH:
		if(flags&KF_SHIFT)
		{
			key = 'C';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'V';
		flags |= KF_ALT;
		break;
	case K_MINUS:
		if(flags&KF_SHIFT)
		{
			key = '0';
			flags &= ~KF_SHIFT;
		}
		else
			key = 'J';
		flags |= KF_ALT;
		break;
	case K_EQUAL:
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

bool eUSPInstance::SpecialKeyDown(const pp::KeyboardInputEvent event)
{
	int key = event.GetKeyCode();
	enum { K_F12 = 123 };
	if(key == K_F12)
	{
		Handler()->OnAction(A_RESET);
		return true;
	}
	else if(key == 'F' && event.GetModifiers()&PP_INPUTEVENT_MODIFIER_CONTROLKEY)
	{
		full_screen.SetFullscreen(!full_screen.IsFullscreen());
		return true;
	}
	return false;
}

bool eUSPInstance::HandleInputEvent(const pp::InputEvent& ev)
{
	switch(ev.GetType())
	{
	case PP_INPUTEVENT_TYPE_KEYDOWN:
		{
			pp::KeyboardInputEvent event(ev);
			if(SpecialKeyDown(event))
				return true;
			int key = event.GetKeyCode();
			dword flags = KF_DOWN|OpJoyKeyFlags();
			if(event.GetModifiers()&PP_INPUTEVENT_MODIFIER_ALTKEY)		flags |= KF_ALT;
			if(event.GetModifiers()&PP_INPUTEVENT_MODIFIER_SHIFTKEY)	flags |= KF_SHIFT;
			TranslateKey(key, flags);
			if(key || flags)
			{
				Handler()->OnKey(key, flags);
				return true;
			}
		}
		break;
	case PP_INPUTEVENT_TYPE_KEYUP:
		{
			pp::KeyboardInputEvent event(ev);
			int key = event.GetKeyCode();
			dword flags = 0;
			if(event.GetModifiers()&PP_INPUTEVENT_MODIFIER_ALTKEY)		flags |= KF_ALT;
			if(event.GetModifiers()&PP_INPUTEVENT_MODIFIER_SHIFTKEY)	flags |= KF_SHIFT;
			TranslateKey(key, flags);
			if(key || flags)
			{
				Handler()->OnKey(key, OpJoyKeyFlags());
				return true;
			}
		}
		break;
	case PP_INPUTEVENT_TYPE_MOUSEMOVE:
		if(mouse_locked)
		{
			pp::MouseInputEvent event(ev);
			pp::Point delta = event.GetMovement();
			Handler()->OnMouse(MA_MOVE, delta.x(), -delta.y());
			return true;
		}
		break;
	case PP_INPUTEVENT_TYPE_MOUSEDOWN:
		if(!mouse_locked)
		{
			LockMouse(callback.NewCallback(&eUSPInstance::DidLockMouse));
		}
		//no break
	case PP_INPUTEVENT_TYPE_MOUSEUP:
		if(mouse_locked)
		{
			pp::MouseInputEvent event(ev);
			Handler()->OnMouse(MA_BUTTON, event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_RIGHT, ev.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN);
			return true;
		}
		break;
	default:
		break;
	}
	return false;
}

}
//namespace xPlatform

namespace pp
{

Module* CreateModule()
{
	struct eUSPModule : public Module
	{
		virtual ~eUSPModule() { glTerminatePPAPI(); }
		virtual bool Init() { return glInitializePPAPI(get_browser_interface()) == GL_TRUE; }
		virtual Instance* CreateInstance(PP_Instance instance) { return new xPlatform::eUSPInstance(instance); }
	};
	return new eUSPModule();
}

}
//namespace pp

#endif//_CHROME_NACL
