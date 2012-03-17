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
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/completion_callback.h"
#include <string>

#pragma once

namespace xPlatform
{

class eURLLoader
{
public:
	struct eCallback
	{
		virtual void OnURLLoadOk(const std::string& url, const char* buffer, size_t size) = 0;
		virtual void OnURLLoadFail(const std::string& url) = 0;
	};
	eURLLoader(pp::Instance* _instance, const std::string& _url, eCallback* cb);
private:
	void OnOpen(int32_t result);
	void ReadBody();
	void OnRead(int32_t result);
	void OnReadData(int32_t num_bytes);
	void OnDone(bool success);

private:
	std::string url;
	eCallback* callback;
	pp::URLRequestInfo url_request;
	pp::URLLoader url_loader;
	enum { READ_BUFFER_SIZE = 4096 };
	char buffer[READ_BUFFER_SIZE];
	std::string data;
	pp::CompletionCallbackFactory<eURLLoader> cc_factory;
};

}
//namespace xPlatform

#endif//_CHROME_NACL
