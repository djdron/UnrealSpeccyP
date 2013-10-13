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

#include "ppapi/c/ppb_gamepad.h"
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
void TranslateKey(int& key, dword& flags);
void UpdateGamepads(const PP_GamepadsSampleData& pads, const PP_GamepadsSampleData& pads_prev);
float OpZoom();

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
	void	Draw();

	//callbacks
	void	Update(int32_t result = 0);
	void	DidLockMouse(int32_t result);

protected:
	eGLContext* gl_context;
	eGLES2* gles2;
	const PPB_Gamepad* gamepad;

	pp::Size size;
	eAudio audio;
	pp::Fullscreen full_screen;
	pp::CompletionCallbackFactory<eUSPInstance> callback;
	bool inited;
	bool mouse_locked;
	struct eMouseDelta
	{
		eMouseDelta() : x(0.0f), y(0.0f) {}
		eMouseDelta(const pp::Point& d, float sx, float sy)
		{
			x = sx*d.x();
			y = sy*d.y();
		}
		eMouseDelta& operator+=(const eMouseDelta& d) { x += d.x; y += d.y; return *this; }
		float x;
		float y;
	};
	eMouseDelta mouse_delta;
	PP_GamepadsSampleData gamepad_data_prev;
};

eUSPInstance::eUSPInstance(PP_Instance instance)
	: pp::Instance(instance), pp::MouseLock(this)
	, gl_context(NULL), gles2(NULL), gamepad(NULL), size(0, 0)
	, full_screen(this), callback(this)
	, inited(false), mouse_locked(false)
{
	pp::Module* module = pp::Module::Get();
	if(module)
		gamepad = static_cast<const PPB_Gamepad*>(module->GetBrowserInterface(PPB_GAMEPAD_INTERFACE));
	memset(&gamepad_data_prev, 0, sizeof(gamepad_data_prev));
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
	if(gamepad)
	{
		PP_GamepadsSampleData gamepad_data;
		gamepad->Sample(pp_instance(), &gamepad_data);
		UpdateGamepads(gamepad_data, gamepad_data_prev);
		gamepad_data_prev = gamepad_data;
	}
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
	gles2->Draw(ePoint(), ePoint(size.width(), size.height()));
	gl_context->Flush();
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
			float z = OpZoom();
			float sx, sy;
			GetScaleWithAspectRatio43(&sx, &sy, size.width(), size.height());
			float scale_x = 320.0f/size.width()/sx/z;
			float scale_y = 240.0f/size.height()/sy/z;
			mouse_delta += eMouseDelta(event.GetMovement(), scale_x, scale_y);
			int dx = mouse_delta.x;
			int dy = mouse_delta.y;
			if(dx || dy)
			{
				mouse_delta.x -= dx;
				mouse_delta.y -= dy;
				Handler()->OnMouse(MA_MOVE, dx, -dy);
			}
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
