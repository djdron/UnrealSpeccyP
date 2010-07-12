/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "log.h"

#ifdef USE_LOG

#include "../std.h"
#include "../platform/io.h"

namespace xLog
{

static char log_path[xIo::MAX_PATH_LEN] = { 0 };
void SetLogPath(const char* _path)
{
	strcpy(log_path, _path);
}
static FILE* log_file = NULL;
static void Close()
{
	fclose(log_file);
	log_file = NULL;
}
static void Open()
{
	char log_name[xIo::MAX_PATH_LEN];
	strcpy(log_name, log_path);
	strcat(log_name, "usp.log");
	log_file = fopen(log_name, "w");
	assert(log_file);
	atexit(Close);
}
void Write(const char* text)
{
	if(!log_file)
		Open();
	fputs(text, log_file);
//	fflush(log_file);
}

}
//namespace xLog

#endif//USE_LOG
