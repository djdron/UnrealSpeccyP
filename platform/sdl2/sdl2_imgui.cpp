/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2024 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_IMGUI

#include "../../3rdparty/imgui/backends/imgui_impl_sdl2.h"
#include "../../3rdparty/imgui/backends/imgui_impl_opengl3.h"

namespace xPlatform
{
namespace xImGui
{

void Init(SDL_Window* window, SDL_GLContext context)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
//  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init();
}

void Done()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Update()
{
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
}

void ProcessEvent(const SDL_Event& e, bool* need_keyboard, bool* need_mouse)
{
	ImGui_ImplSDL2_ProcessEvent(&e);
	ImGuiIO& io = ImGui::GetIO();
	*need_keyboard = io.WantCaptureKeyboard;
	*need_mouse = io.WantCaptureMouse;
}

}
//namespace xImGui
}
//namespace xPlatform

#else//USE_IMGUI

namespace xPlatform
{
namespace xImGui
{
void Init(SDL_Window* window, SDL_GLContext context) {}
void Done() {}
void Update() {}
void ProcessEvent(const SDL_Event& e, bool* need_keyboard, bool* need_mouse) {}

}
//namespace xImGui
}
//namespace xPlatform

#endif//USE_IMGUI
#endif//USE_SDL2
