/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_EMSCRIPTEN

#include "../../tools/options.h"
#include "../platform.h"
#include "../io.h"
#include <emscripten.h>
#include <string>

namespace xPlatform
{

#ifdef USE_WEB
namespace xWeb
{

std::string GetURL(const char* _url)
{
	using namespace std;
	const char* file_name = xIo::ProfilePath("download.tmp");
	emscripten_wget(_url, file_name);

	string data;
	FILE* f = fopen(file_name, "rb");
	if(!f)
		return string();
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	data.resize(size, 0);
	size_t r = fread((void*)data.c_str(), 1, size, f);
	fclose(f);
	if(r != size)
		return string();
	return data;
}

}
#endif//USE_WEB


void Loop1();

void Loop()
{
	using namespace xOptions;
	struct eOptionBX : public eOptionB
	{
		void Unuse() { customizable = false; storeable = false; }
	};
	eOptionBX* o = (eOptionBX*)eOptionB::Find("quit");
	SAFE_CALL(o)->Unuse();

	eOptionInt* zoom = (eOptionInt*)eOptionB::Find("zoom");
	SAFE_CALL(zoom)->Set(1); // fill screen

	EM_ASM
	(
		Module.onReady();
	);
	emscripten_set_main_loop(xPlatform::Loop1, 0, true);
}

void Done();

}
//namespace xPlatform

extern "C"
{

EMSCRIPTEN_KEEPALIVE
void OpenFileData(const char* url, const char* data, int size)
{
	xPlatform::Handler()->OnOpenFile(url, data, size);
}

static void OnLoadOK(void* url, void* data, int size)
{
	xPlatform::Handler()->OnOpenFile((const char*)url, data, size);
	free(url);
}

static void OnLoadFail(void* url)
{
	free(url);
}

EMSCRIPTEN_KEEPALIVE
void OpenFile(const char* url)
{
	emscripten_async_wget_data(url, strdup(url), OnLoadOK, OnLoadFail);
}

EMSCRIPTEN_KEEPALIVE
void ExitLoop()
{
	emscripten_cancel_main_loop();
	xPlatform::Done();
	emscripten_force_exit(0);
}

EMSCRIPTEN_KEEPALIVE
void OnCommand(const char* cmd)
{
	using namespace std;
	string m(cmd);
	static const string joystick("joystick:");
	static const string zoom("zoom:");
	static const string filtering("filtering:");
	static const string black_and_white("black and white:");
	static const string gigascreen("gigascreen:");
	if(m.length() > joystick.length() && m.substr(0, joystick.length()) == joystick)
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
		string v = m.substr(filtering.length());
		eOption<bool>* op_filtering = eOption<bool>::Find("filtering");
		SAFE_CALL(op_filtering)->Value(v.c_str());
	}
	else if(m.length() > black_and_white.length() && m.substr(0, black_and_white.length()) == black_and_white)
	{
		using namespace xOptions;
		string v = m.substr(black_and_white.length());
		eOption<bool>* op_black_and_white = eOption<bool>::Find("black and white");
		SAFE_CALL(op_black_and_white)->Value(v.c_str());
	}
	else if(m.length() > gigascreen.length() && m.substr(0, gigascreen.length()) == gigascreen)
	{
		using namespace xOptions;
		string v = m.substr(gigascreen.length());
		eOption<bool>* op_gigascreen = eOption<bool>::Find("gigascreen");
		SAFE_CALL(op_gigascreen)->Value(v.c_str());
	}
	else if(m == "reset")
	{
		using namespace xPlatform;
		Handler()->OnAction(A_RESET);
	}
}

}
//extern "C"

#endif//USE_EMSCRIPTEN
