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

class eOption : public eList<eOption>
{
public:
	enum eType { OT_INT, OT_BOOL, OT_STRING,  };
	eOption() : type(OT_INT), customizable(true), storeable(true), value_int(0) {}

	eType	Type() const { return type; }
	bool	Customizable() const { return customizable; }
	bool	Storeable() const { return storeable; }

	static eOption* Find(const char* name)
	{
		for(eOption* o = First(); o; o = o->Next())
		{
			if(!strcmp(name, o->Name()))
				return o;
		}
		return NULL;
	}

	int			ValueInt() const	{ assert(type == OT_INT); return value_int; }
	bool		ValueBool() const	{ assert(type == OT_BOOL); return value_bool; }
	const char*	ValueStr() const	{ assert(type == OT_STRING); return value_str; }

	const char*	Value() const
	{
		if(type == OT_STRING)
			return ValueStr();
		const char** vals = Values();
		if(!vals)
			return NULL;
		switch(type)
		{
		case OT_INT:	return vals[ValueInt()];
		case OT_BOOL:	return vals[ValueBool() ? 1 : 0];
		default:		assert(true);
		}
		return NULL;
	}
	void Value(const char* v)
	{
		if(type == OT_STRING)
		{
			//@todo : copy string data ???
			ValueStr(v);
			return;
		}
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
		if(i >= 0)
		{
			switch(type)
			{
			case OT_INT:	ValueInt(i);		break;
			case OT_BOOL:	ValueBool(i != 0);	break;
			default:		assert(true);		break;
			}
		}
	}

	void ValueInt(int v)		{ type = OT_INT; value_int = v; }
	void ValueBool(bool v)		{ type = OT_BOOL; value_bool = v; }
	void ValueStr(const char* v){ type = OT_STRING; value_str = v; }

	virtual const char* Name() const = 0;
	virtual const char** Values() const { return NULL; }
	virtual void Change(bool next = true) {}
	virtual void Apply() {}

protected:
	eType	type;
	bool 	customizable;
	bool	storeable;
	union
	{
		int			value_int;
		bool		value_bool;
		const char* value_str;
	};
};

struct eOptionBool : public eOption
{
	eOptionBool() { ValueBool(false); }
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "on", NULL };
		return values;
	}
};

void Load();
void Store();

}
//namespace xOptions

#endif//__OPTION_H__
