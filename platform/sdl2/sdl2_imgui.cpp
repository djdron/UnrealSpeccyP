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

static void ProcessReplayInfo()
{
	dword frame_current = 0, frames_total = 0, frames_cached = 0;
	if(!xPlatform::Handler()->GetReplayProgress(&frame_current, &frames_total, &frames_cached) || !frames_total)
		return;

	char buf[16];
	auto TimeToString = [&buf](int seconds)
	{
		int h = seconds / 3600;
		int m = (seconds % 3600) / 60;
		int s = seconds % 60;
		if(h > 0)
			snprintf(buf, sizeof buf, "%02d:%02d:%02d", h, m, s);
		else
			snprintf(buf, sizeof buf, "%02d:%02d", m, s);
		return buf;
	};

	float alpha = 1.0f;

	static ImVec2 smax = ImGui::CalcTextSize("88:88:88");
	static ImVec2 smin = ImGui::CalcTextSize("88:88");
	static float scenter = (smax.x - smin.x)/2.0f;

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
	ImVec2 rect = draw_list->GetClipRectMax();

	const char* time_current = TimeToString(frame_current/50);
	float dx_current = (strlen(time_current) == 5) ? scenter : 0.0f;
	draw_list->AddText(ImVec2(10 + dx_current, rect.y - smax.y*2), ImColor(1.0f, 1.0f, 1.0f, 1.0f*alpha), time_current);

	const char* time_total = TimeToString(frames_total/50);
	float dx_total = (strlen(time_total) == 5) ? scenter : 0.0f;
	draw_list->AddText(ImVec2(rect.x - 10 - smax.x + dx_total, rect.y - smax.y*2), ImColor(1.0f, 1.0f, 1.0f, 1.0f*alpha), time_total);

	float p = (float)frame_current/frames_total;
	int width = rect.x - 40 - smax.x*2;
	int px = 20 + smax.x;
	int py = smax.y*3/2;
	int psx = (int)(width*p);
	draw_list->AddRectFilled(ImVec2(px, rect.y - py - 2), ImVec2(px + psx, rect.y - py + 3), ImColor(0.5f, 0.0f, 0.0f, 0.8f*alpha));
	int psxc = psx;
	if(frame_current + frames_cached < frames_total)
	{
		float pc = (float)(frame_current + frames_cached)/frames_total;
		psxc = (int)(width*pc);
		draw_list->AddRectFilled(ImVec2(px + psx, rect.y - py - 2), ImVec2(px + psxc, rect.y - py + 3), ImColor(0.4f, 0.4f, 0.4f, 0.8f*alpha));
	}
	draw_list->AddRectFilled(ImVec2(px + psxc, rect.y - py - 2), ImVec2(px + width, rect.y - py + 3), ImColor(0.2f, 0.2f, 0.2f, 0.8f*alpha));
	draw_list->AddCircleFilled(ImVec2(px + psx, rect.y - py + 1), 5, ImColor(1.0f, 1.0f, 1.0f, 0.8f*alpha));
}

static void Process()
{
	static bool show_demo_window = true;
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	static bool show_test = true;
	if(show_test)
	{
		if(ImGui::Begin("test", &show_test))
		{
		}
		ImGui::End();
	}

	ProcessReplayInfo();
}

void Update()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	Process();

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
