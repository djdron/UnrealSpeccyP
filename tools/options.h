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

class TiXmlElement;

namespace xOptions
{

class eOptionB;

class eRootOptionB : public eList<eRootOptionB>
{
public:
	virtual int Order() const { return 0; }
	virtual eOptionB* OptionB() = 0;
	static eOptionB* Find(const char* name);
	static void Apply();
	static void Load(TiXmlElement* owner);
	static void Store(TiXmlElement* owner);
protected:
	eRootOptionB() {}
};

template<class T> struct eRootOption : public eRootOptionB, public T
{
	virtual eOptionB* OptionB() { return (eOptionB*)this; }
	static T* Find(const char* name) { return (T*)eRootOptionB::Find(name); }
};

class eOptionB
{
public:
	eOptionB() : next(NULL), sub_options(NULL), customizable(true), storeable(true), changed(false) {}
	virtual ~eOptionB() {}

	eOptionB* Next() { return next; }
	eOptionB* SubOptions() { return sub_options; }

	bool Customizable() const { return customizable; }
	bool Storeable() const { return storeable; }
	bool Changed() const { return changed; }

	virtual const char* Name() const = 0;
	virtual const char*	Value() const { return NULL; }
	virtual void Value(const char* v) {}
	virtual void Change(bool next = true) { changed = true; }
	bool Option(eOptionB& o);
	bool Option(eOptionB* o) { return o ? Option(*o) : false; }
	void Apply();
	void Load(TiXmlElement* owner);
	void Store(TiXmlElement* owner);
	static const char* OptNameToXmlName(const char* name);
	static const char* XmlNameToOptName(const char* name);
protected:
	virtual const char** Values() const { return NULL; }
	virtual void OnOption() {}
	eOptionB* Find(const char* name) const;
public:
	static bool loading;
protected:
	eOptionB* next;
	eOptionB* sub_options;
	bool customizable;
	bool storeable;
	bool changed;
	static char buf[256];
};

template<class T> class eOption : public eOptionB
{
public:
	operator const T&() const { return value; }
	virtual void Set(const T& v) { value = v; changed = true; }
protected:
	T	value;
};

class eOptionInt : public eOption<int>
{
	typedef eOption<int> eInherited;
public:
	eOptionInt() { Set(0); }
protected:
	void Change(int last, bool next = true);
	virtual const char*	Value() const;
	virtual void Value(const char* v);
};

class eOptionBool : public eOption<bool>
{
	typedef eOption<bool> eInherited;
public:
	eOptionBool() { Set(false); }
protected:
	virtual const char*	Value() const;
	virtual void Value(const char* v);
	virtual const char** Values() const;
	virtual void Change(bool next = true) { Set(!value); eInherited::Change(); }
};

struct eOptionString : public eOption<const char*>
{
	typedef eOption<const char*> eInherited;
	eOptionString() : alloc_size(32) { value = new char[alloc_size]; Value(""); }
	virtual ~eOptionString() { SAFE_DELETE_ARRAY(value); }
	virtual const char*	Value() const { return value; }
	virtual void Value(const char* v) { Set(v); }
	virtual void Set(const char*& v);
	int alloc_size;
};

void Load();
void Store();

}
//namespace xOptions

#endif//__OPTION_H__
