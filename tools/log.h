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

#ifndef __LOG_H__
#define __LOG_H__

#pragma once

//#define USE_LOG

namespace xLog
{

#ifdef USE_LOG
void SetLogPath(const char* path);
void Write(const char* text);
#else//USE_LOG
inline void SetLogPath(const char* path) {}
#endif//USE_LOG

}
//namespace xLog

#ifdef USE_LOG
#define _LOG(a) xLog::Write(a)
#else//USE_LOG
#define _LOG(a)
#endif//USE_LOG

#endif//__LOG_H__
