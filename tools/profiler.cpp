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

#include "profiler.h"
#include "log.h"

#ifdef USE_PROFILER

namespace xProfiler
{

//=============================================================================
//	eSection::eSection
//-----------------------------------------------------------------------------
eSection::eSection(const char* _name) : name(_name), entry_count(0)
{
}
//=============================================================================
//	eSection::End
//-----------------------------------------------------------------------------
void eSection::End()
{
	eTime t = start_tick.Passed();
	time_total += t;
	if(!entry_count)
	{
		time_min = time_max = t;
	}
	else
	{
		if(time_min > t)
			time_min = t;
		if(time_max < t)
			time_max = t;
	}
	++entry_count;
}
//=============================================================================
//	eSection::Reset
//-----------------------------------------------------------------------------
void eSection::Reset()
{
	time_total = time_min = time_max = 0;
	entry_count = 0;
}
//=============================================================================
//	eSection::Dump
//-----------------------------------------------------------------------------
const char* eSection::Dump()
{
	static char dump[1024];
	float average = entry_count ? time_total.Ms()/entry_count : 0;
	sprintf(dump, "%8s: %.2f/%.2f/%.2f (%u)", name, average, time_min.Ms(), time_max.Ms(), entry_count);
	return dump;
}
//=============================================================================
//	eSection::DumpAll
//-----------------------------------------------------------------------------
void eSection::DumpAll()
{
	_LOG("Profiler dump:\n");
	for(eSection* i = First(); i; i = i->Next())
	{
		_LOG(i->Dump());
		_LOG("\n");
	}
}
//=============================================================================
//	eSection::ResetAll
//-----------------------------------------------------------------------------
void eSection::ResetAll()
{
	for(eSection* i = First(); i; i = i->Next())
	{
		i->Reset();
	}
}

}
//namespace xProfiler

#endif//USE_PROFILER
