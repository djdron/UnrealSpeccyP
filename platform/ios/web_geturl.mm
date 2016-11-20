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

#import <string>
#import "Foundation/Foundation.h"

namespace xPlatform
{
namespace xWeb
{

std::string GetURL(const char* path)
{
	NSURLRequest* urlRequest = [NSURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:path]]];
	NSURLResponse* response = nil;
	NSError* error = nil;
	NSData* data = [NSURLConnection sendSynchronousRequest:urlRequest returningResponse:&response error:&error];
	if(error != nil)
		return std::string();

	if(response != nil && [response isKindOfClass:[NSHTTPURLResponse class]])
	{
		NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
		if(httpResponse.statusCode != 200)
			return std::string();
	}
	std::string file_data;
	file_data.resize(data.length);
	[data getBytes:(void*)file_data.c_str() length:file_data.length()];
	return file_data;
}

}
//namespace xWeb
}
//namespace xPlatform
