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

#ifndef __PROFILER_H__
#define __PROFILER_H__

#include "../std.h"
#include "list.h"
#include "tick.h"

#pragma once

//#define USE_PROFILER

#ifdef USE_PROFILER

namespace xProfiler
{

class eSection : public eList<eSection>
{
public:
	eSection(const char* _name);
	void	Begin()
	{
		start_tick.SetCurrent();
	}
	void	End();
	const char* Dump();
	void	Reset();

	static void	DumpAll();
	static void	ResetAll();

protected:
	const char* name;
	eTime	time_total;
	eTime	time_min;
	eTime	time_max;
	eTick	start_tick;
	int		entry_count;
};

class eSectionAuto
{
public:
	eSectionAuto(eSection& _s) : section(_s) { section.Begin(); }
	~eSectionAuto() { section.End(); }
protected:
	eSection& section;
};

}
//namespace xProfiler

#define PROFILER_DECLARE(name) static xProfiler::eSection profile_section_##name(#name);
#define PROFILER_BEGIN(name) profile_section_##name.Begin();
#define PROFILER_END(name) profile_section_##name.End();
#define PROFILER_SECTION(name) xProfiler::eSectionAuto profile_section_auto_##name(profile_section_##name);
#define PROFILER_DUMP xProfiler::eSection::DumpAll();

#else//USE_PROFILER

#define PROFILER_DECLARE(name)
#define PROFILER_BEGIN(name)
#define PROFILER_END(name)
#define PROFILER_SECTION(name)
#define PROFILER_DUMP

#endif//USE_PROFILER

#endif//__PROFILER_H__
