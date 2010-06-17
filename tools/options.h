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

#ifndef __OPTION_H__
#define __OPTION_H__

#include "../std.h"
#include "list.h"

#pragma once

namespace xOptions
{

class eOptionB : public eList<eOptionB>
{
public:
	eOptionB() : customizable(true), storeable(true) {}
	virtual ~eOptionB() {}

	bool	Customizable() const { return customizable; }
	bool	Storeable() const { return storeable; }

	virtual const char* Name() const = 0;
	virtual const char*	Value() const { return NULL; }
	virtual void Value(const char* v) {}
	virtual const char** Values() const { return NULL; }
	virtual void Change(bool next = true) {}
	virtual void Apply() {}

	static eOptionB* Find(const char* name)
	{
		for(eOptionB* o = First(); o; o = o->Next())
		{
			if(!strcmp(name, o->Name()))
				return o;
		}
		return NULL;
	}

protected:
	bool 	customizable;
	bool	storeable;
};

template<class T> class eOption : public eOptionB
{
public:
	operator const T&() const { return value; }
	virtual void Set(const T& v) { value = v; }
	static eOption* Find(const char* name) { return (eOption*)eOptionB::Find(name); }

protected:
	T	value;
};

class eOptionInt : public eOption<int>
{
protected:
	void Change(int f, int l, bool next = true)
	{
		if(next)
		{
			Set(self + 1);
			if(self >= l)
				Set(f);
		}
		else
		{
			Set(self - 1);
			if(self < f)
				Set(l - 1);
		}
	}
	virtual const char*	Value() const
	{
		const char** vals = Values();
		if(!vals)
			return NULL;
		return vals[value];
	}
	virtual void Value(const char* v)
	{
		const char** vals = Values();
		if(!vals)
			return;
		int i = -1;
		for(; *vals; ++vals)
		{
			++i;
			if(!strcmp(*vals, v))
				break;
		}
		value = i;
	}
};

struct eOptionBool : public eOption<bool>
{
	virtual void Change(bool next = true) { Set(!value); }
	virtual const char*	Value() const
	{
		const char** vals = Values();
		if(!vals)
			return NULL;
		return vals[value ? 1 : 0];
	}
	virtual void Value(const char* v)
	{
		const char** vals = Values();
		if(!vals)
			return;
		if(!strcmp(v, vals[0]))
			value = false;
		else if(!strcmp(v, vals[1]))
			value = true;
	}
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "on", NULL };
		return values;
	}
};

struct eOptionString : public eOption<const char*>
{
	eOptionString() : alloc_size(0) {}
	virtual ~eOptionString() { SAFE_DELETE_ARRAY(value); }
	virtual const char*	Value() const
	{
		return value ? value : "";
	}
	virtual void Value(const char* v)
	{
		int s = strlen(v) + 1;
		if(!value || alloc_size < s)
		{
			SAFE_DELETE_ARRAY(value);
			value = new char[s];
			alloc_size = s;
		}
		strcpy(const_cast<char*>(value), v);
	}
	virtual void Set(const char*& v) { Value(v); }
	int alloc_size;
};

void Load();
void Store();

}
//namespace xOptions

#endif//__OPTION_H__
