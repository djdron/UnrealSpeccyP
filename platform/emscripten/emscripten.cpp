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

}
//extern "C"

#endif//USE_EMSCRIPTEN
