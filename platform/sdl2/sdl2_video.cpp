/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2017 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_SDL2

#include <SDL.h>
#include "../gles2/gles2.h"
#include "../../tools/options.h"

#ifdef USE_IMGUI
#include "../../3rdparty/imgui/backends/imgui_impl_sdl2.h"
#include "../../3rdparty/imgui/backends/imgui_impl_opengl3.h"
#endif//USE_IMGUI

namespace xPlatform
{

SDL_Window* window = NULL;
static SDL_GLContext context = NULL;
static eGLES2* gles2 = NULL;

class eOptionWindowState : public xOptions::eOptionString
{
public:
	eOptionWindowState() { customizable = false; }
	virtual const char* Name() const { return "window state"; }

	bool Get(ePoint* position, ePoint* size, bool* maximized) const
	{
		ePoint p(SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED);
		ePoint s(960, 720);
		int m = 0;
		bool ok = sscanf(Value(), FormatStr(), &p.x, &p.y, &s.x, &s.y, &m) == 5;
		if(position)
			*position = p;
		if(size)
			*size = s;
		if(maximized)
			*maximized = m != 0;
		return ok;
	}
	void Set(const ePoint* position, const ePoint* size, const bool* maximized)
	{
		ePoint p, s;
		bool m;
		Get(&p, &s, &m);
		if(position)
			p = *position;
		if(size)
			s = *size;
		if(maximized)
			m = *maximized;
		char buf[512];
		sprintf(buf, FormatStr(), p.x, p.y, s.x, s.y, m ? 1 : 0);
		Value(buf);
	}
	void Update()
	{
		Uint32 flags = SDL_GetWindowFlags(window);
		if((flags&SDL_WINDOW_FULLSCREEN))
			return;
		if(!(flags&SDL_WINDOW_MAXIMIZED) && !(flags&SDL_WINDOW_MINIMIZED))
		{
			ePoint p;
			SDL_GetWindowPosition(window, &p.x, &p.y);
			ePoint s;
			SDL_GetWindowSize(window, &s.x, &s.y);
			bool m = false;
			Set(&p, &s, &m);
		}
		else
		{
			bool m = (flags&SDL_WINDOW_MAXIMIZED) != 0;
			Set(NULL, NULL, &m);
		}
	}

private:
	const char* FormatStr() const { return "position(%d, %d); size(%d, %d); maximized(%d)"; }
} op_window_state;

static struct eOptionFullScreen : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "full screen"; }
	virtual int Order() const { return 32; }
	virtual void Set(const bool& v)
	{
		eOptionBool::Set(v);
		Apply();
	}
	virtual void Apply()
	{
		SDL_SetWindowFullscreen(window, (*this) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	}
	void Update()
	{
		bool fs = (SDL_GetWindowFlags(window)&SDL_WINDOW_FULLSCREEN) != 0;
		if(*this != fs)
			Set(fs);
	}
} op_full_screen;

bool InitVideo()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1");

	ePoint pos, size;
	bool maximized;
	op_window_state.Get(&pos, &size, &maximized);
	Uint32 flags = SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI;
	if(maximized)
		flags |= SDL_WINDOW_MAXIMIZED;
	if(op_full_screen)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	window = SDL_CreateWindow(Handler()->WindowCaption(), pos.x, pos.y, size.x, size.y, flags);
	if(!window)
		return false;
	context = SDL_GL_CreateContext(window);
	if(!context)
		return false;

	SDL_GL_MakeCurrent(window, context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

#ifdef _LINUX
	#include "../../build/linux/icon.c"
	SDL_Surface* icon_sufrace = SDL_CreateRGBSurfaceFrom((void*)icon.pixel_data, icon.width, icon.height,
		icon.bytes_per_pixel * 8, icon.bytes_per_pixel*icon.width, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_SetWindowIcon(window, icon_sufrace);
	SDL_FreeSurface(icon_sufrace);
#endif//_LINUX

#ifdef USE_IMGUI
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init();
#endif//USE_IMGUI

	gles2 = eGLES2::Create();
	return true;
}

void DoneVideo()
{
	SAFE_DELETE(gles2);

#ifdef USE_IMGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
#endif//USE_IMGUI

	if(context)
	{
		SDL_GL_DeleteContext(context);
		context = NULL;
	}
	if(window)
	{
		SDL_DestroyWindow(window);
		window = NULL;
	}
}

void UpdateScreen()
{
	op_window_state.Update();
	op_full_screen.Update();

	ePoint s;
	SDL_GL_GetDrawableSize(window, &s.x, &s.y);
	gles2->Draw(ZERO, s);

#ifdef USE_IMGUI
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

	static bool show_demo_window = true;
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if(show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif//USE_IMGUI

	SDL_GL_SwapWindow(window);
}

}
//namespace xPlatform

#endif//USE_SDL2
