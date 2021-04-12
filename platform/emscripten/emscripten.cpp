/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
#include <SDL.h>
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

bool Init();
void Loop1();
void Done();

static void Setup()
{
	using namespace xOptions;
	struct eOptionBX : public eOptionB
	{
		void Unuse() { customizable = false; storeable = false; }
	};
	eOptionBX* o = (eOptionBX*)eOptionB::Find("quit");
	SAFE_CALL(o)->Unuse();
}

}
//namespace xPlatform

EMSCRIPTEN_KEEPALIVE
extern "C"
void mainFS()
{
	if(!xPlatform::Init())
	{
		xPlatform::Done();
		return;
	}
	EM_ASM
	(
		Module.onReady();
	);
	emscripten_set_main_loop(xPlatform::Loop1, 0, false);
}

int main(int argc, char* argv[])
{
	xPlatform::Setup();
	EM_ASM
	(
		FS.mkdir('/cache');
		FS.mount(IDBFS, {}, '/cache');
		FS.syncfs(true, function(err) { ccall('mainFS', 'v'); });
	);
	emscripten_exit_with_live_runtime();
	return 0;
}

namespace xIo
{
void SyncFS()
{
	xOptions::Store();
	EM_ASM
	(
		FS.syncfs(function(err) {});
	);
}

}
//namespace xIo

extern "C"
{

static void OnLoadOK(const char* name)
{
	xIo::SyncFS();
	xPlatform::Handler()->OnOpenFile(name);
}

static void OnLoadFail(const char* name)
{
}

EMSCRIPTEN_KEEPALIVE
void OpenFile(const char* url)
{
	std::string name = url;
	auto p = name.rfind('/');
	if(p != std::string::npos)
		name.erase(0, p + 1);
	emscripten_async_wget(url, xIo::ProfilePath(name.c_str()), OnLoadOK, OnLoadFail);
}

EMSCRIPTEN_KEEPALIVE
void ExitLoop()
{
	emscripten_cancel_main_loop();
	xPlatform::Done();
	emscripten_force_exit(0);
}

EMSCRIPTEN_KEEPALIVE
void OnCommand(const char* _cmd)
{
	using namespace std;
	string cmd(_cmd);
	auto pos = cmd.rfind('=');
	if(pos == string::npos)
	{
		if(cmd == "reset")
		{
			using namespace xPlatform;
			Handler()->OnAction(A_RESET);
		}
		return;
	}
	string option = cmd.substr(0, pos);
	string value = cmd.substr(pos + 1);
	auto* opt = xOptions::eOptionB::Find(option.c_str());
	if(opt)
	{
		opt->Value(value.c_str());
		opt->Apply();
	}
}

}
//extern "C"

#endif//USE_EMSCRIPTEN
