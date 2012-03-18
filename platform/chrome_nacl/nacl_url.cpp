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

#include "nacl_url.h"

using namespace std;

namespace xPlatform
{

eURLLoader::eURLLoader(pp::Instance* _instance, const string& _url, eCallback* cb)
	: url(_url), callback(cb), url_request(_instance), url_loader(_instance), cc_factory(this)
{
	url_request.SetURL(url);
	url_request.SetMethod("GET");
//	url_request.SetProperty(PP_URLREQUESTPROPERTY_ALLOWCROSSORIGINREQUESTS, pp::Var(true));
	pp::CompletionCallback cc = cc_factory.NewRequiredCallback(&eURLLoader::OnOpen);
	url_loader.Open(url_request, cc);
}
void eURLLoader::OnOpen(int32_t result)
{
	if(result == PP_OK)
		ReadBody();
	else
		OnDone(false);
}
void eURLLoader::ReadBody()
{
	pp::CompletionCallback cc = cc_factory.NewOptionalCallback(&eURLLoader::OnRead);
	int32_t result = PP_OK;
	do
	{
		result = url_loader.ReadResponseBody(buffer, sizeof(buffer), cc);
		if(result > 0)
			OnReadData(result);
	}
	while(result > 0);
	if(result != PP_OK_COMPLETIONPENDING)
	{
		cc.Run(result);
	}
}
void eURLLoader::OnRead(int32_t result)
{
	if(result == PP_OK)
		OnDone(true);
	else if(result > 0)
	{
		OnReadData(result);
		ReadBody();
	}
	else
		OnDone(false);
}
void eURLLoader::OnReadData(int32_t num_bytes)
{
	if(num_bytes <= 0)
		return;
	if(num_bytes > READ_BUFFER_SIZE)
		num_bytes = READ_BUFFER_SIZE;
	data.reserve(data.size() + num_bytes);
	data.insert(data.end(), buffer, buffer + num_bytes);
}
void eURLLoader::OnDone(bool success)
{
	if(success)
		callback->OnURLLoadOk(url, data.c_str(), data.size());
	else
		callback->OnURLLoadFail(url);
	delete this;
}

}
//namespace xPlatform

#endif//_CHROME_NACL
