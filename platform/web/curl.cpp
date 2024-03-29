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

#include "../platform.h"

#ifdef USE_CURL

#include <curl/curl.h>
#include <string>

namespace xPlatform
{
namespace xWeb
{

static struct eCURL_Init
{
	eCURL_Init() { curl_global_init(CURL_GLOBAL_ALL); }
	~eCURL_Init() { curl_global_cleanup(); }
} curl_init;

static size_t _write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	std::string* file_data = (std::string*)userdata;
	*file_data += std::string(ptr, size*nmemb);
	return size*nmemb;
}

std::string GetURL(const char* path, const char* header)
{
	CURL* curl = curl_easy_init();
	curl_slist* slist = NULL;
	if(header)
	{
		slist = curl_slist_append(slist, header);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
	}
	curl_easy_setopt(curl, CURLOPT_URL, path);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
#ifdef _UWP
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif//_UWP
	std::string file_data;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file_data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_perform(curl);
	if(slist)
		curl_slist_free_all(slist);
	curl_easy_cleanup(curl);
	return file_data;
}

}
//namespace xWeb
}
//namespace xPlatform

#endif//USE_CURL
