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

#ifndef	__TYPE_REGISTRATOR_H__
#define	__TYPE_REGISTRATOR_H__

#pragma once

#define DECLARE_REGISTRATOR(list, cls) \
struct list : public eList<list> \
{ \
	static cls* Create(const char* id) \
	{ \
		list* t = Find(id); \
		return t ? t->Create() : NULL; \
	} \
protected: \
	virtual const char* Id() const = 0; \
	virtual cls* Create() const = 0; \
	static list* Find(const char* id) \
	{ \
		for(list* o = o->First(); o; o = o->Next()) \
		{ \
			if(!strcmp(o->Id(), id)) \
				return o; \
		} \
		return NULL; \
	} \
};

#define REGISTER_TYPE(list, cls, id) \
struct _##cls##Info : public list \
{ \
protected: \
	virtual const char* Id() const { return id; } \
	virtual cls* Create() const { return new cls; } \
} _##cls##_info;

#endif//__TYPE_REGISTRATOR_H__
