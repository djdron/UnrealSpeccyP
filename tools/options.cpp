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

#include "options.h"
#include "../platform/platform.h"

#ifdef USE_CONFIG
#include "../platform/io.h"
#include "tinyxml/tinyxml.h"
#endif//USE_CONFIG

namespace xOptions
{

bool loading = false;

static char buf[256];
static const char* OptNameToXmlName(const char* name)
{
	strcpy(buf, name);
	for(char* b = buf; *b; ++b)
	{
		if(*b == ' ')
			*b = '_';
	}
	return buf;
}
static const char* XmlNameToOptName(const char* name)
{
	strcpy(buf, name);
	for(char* b = buf; *b; ++b)
	{
		if(*b == '_')
			*b = ' ';
	}
	return buf;
}

void eOptionB::Apply()
{
	sub_options = NULL;
	OnOption();
	changed = false;
}
bool eOptionB::Option(eOptionB& o)
{
	eOptionB* last = sub_options;
	for(eOptionB* p = last; p; p = p->Next())
	{
		last = p;
	}
	if(last)
		last->next = &o;
	else
		sub_options = &o;
	o.next = NULL;
	bool res = o.Changed();
	if(!loading)
		o.Apply();
	return res;
}
eOptionB* eOptionB::Find(const char* name) const
{
	const char* sub_name = strchr(name, '/');
	int size = sub_name ? sub_name - name : strlen(name);
	for(eOptionB* o = sub_options; o; o = o->Next())
	{
		if(!strncmp(name, o->Name(), size))
			return sub_name ? o->Find(sub_name + 1) : o;
	}
	return NULL;
}
void eOptionB::Load(TiXmlElement* owner)
{
	const char* v = owner->GetText();
	Value(v ? v : "");
	Apply();
	for(TiXmlElement* xe = owner->FirstChildElement(); xe; xe = xe->NextSiblingElement())
	{
		const char* id = xe->Value();
		eOptionB* o = Find(XmlNameToOptName(id));
		SAFE_CALL(o)->Load(xe);
	}
}
void eOptionB::Store(TiXmlElement* owner)
{
	if(!(storeable && Value()) && !sub_options)
		return;
	TiXmlElement* xe = new TiXmlElement(OptNameToXmlName(Name()));
	owner->LinkEndChild(xe);
	if(storeable && Value())
	{
		xe->LinkEndChild(new TiXmlText(Value()));
	}
	for(eOptionB* o = sub_options; o; o = o->Next())
	{
		if(!o->Storeable())
			continue;
		o->Store(xe);
	}
}

struct eOA : public eRootOptionB // access to protected members hack
{
	static void SortByOrder()
	{
		bool swapped;
		do
		{
			swapped = false;
			for(eRootOptionB* a = First(), *pa = NULL; a; pa = a, a = a->Next())
			{
				for(eRootOptionB* b = a->Next(), *pb = a; b; pb = b, b = b->Next())
				{
					if(b->Order() < a->Order())
					{
						Swap((eOA*)pa, (eOA*)a, (eOA*)pb, (eOA*)b);
						swapped = true;
						break;
					}
				}
				if(swapped)
					break;
			}
		} while(swapped);
	}
	static void Swap(eOA* pa, eOA* a, eOA* pb, eOA* b)
	{
		eRootOptionB* n = a->next;
		a->next = b->next;
		if(a != pb)
			b->next = n;
		else
			b->next = a;
		if(pa)
			pa->next = b;
		else
			_First() = b;
		if(a != pb)
			pb->next = a;
	}
};

void eOptionInt::Change(int last, bool next)
{
	if(next)
	{
		Set(self + 1);
		if(self >= last)
			Set(0);
	}
	else
	{
		Set(self - 1);
		if(self < 0)
			Set(last - 1);
	}
	eInherited::Change(next);
}
const char*	eOptionInt::Value() const
{
	const char** vals = Values();
	if(!vals)
		return NULL;
	return vals[value];
}
void eOptionInt::Value(const char* v)
{
	const char** vals = Values();
	if(!vals)
		return;
	int i = -1;
	for(; *vals; ++vals)
	{
		++i;
		if(!strcmp(*vals, v))
		{
			Set(i);
			break;
		}
	}
}

const char*	eOptionBool::Value() const
{
	const char** vals = Values();
	if(!vals)
		return NULL;
	return vals[value ? 1 : 0];
}
void eOptionBool::Value(const char* v)
{
	return;
	const char** vals = Values();
	if(!vals)
		return;
	if(!strcmp(v, vals[0]))
		Set(false);
	else if(!strcmp(v, vals[1]))
		Set(true);
}
const char** eOptionBool::Values() const
{
	static const char* values[] = { "off", "on", NULL };
	return values;
}

void eOptionString::Set(const char*& v)
{
	int s = strlen(v) + 1;
	if(!value || alloc_size < s)
	{
		SAFE_DELETE_ARRAY(value);
		value = new char[s];
		alloc_size = s;
	}
	strcpy(const_cast<char*>(value), v);
	eInherited::Set(value);
}

eOptionB* Find(const char* name)
{
	const char* sub_name = strchr(name, '/');
	int size = sub_name ? sub_name - name : strlen(name);
	for(eRootOptionB* r = eRootOptionB::First(); r; r = r->Next())
	{
		eOptionB* o = r->OptionB();
		if(!strncmp(name, o->Name(), size))
			return sub_name ? o->Find(sub_name + 1) : o;
	}
	return NULL;
}
void Apply()
{
	for(eRootOptionB* r = eRootOptionB::First(); r; r = r->Next())
	{
		r->OptionB()->Apply();
	}
}

#ifdef USE_CONFIG
static const char* FileName() { return xIo::ProfilePath("unreal_speccy_portable.xml"); }
void Init()
{
	eOA::SortByOrder();
	Apply();
	TiXmlDocument doc;
	if(!doc.LoadFile(FileName()))
		return;
	TiXmlElement* root = doc.RootElement();
	if(!root)
		return;
	TiXmlElement* owner = root->FirstChildElement("Options");
	if(!owner)
		return;
	loading = true;
	for(TiXmlElement* xe = owner->FirstChildElement(); xe; xe = xe->NextSiblingElement())
	{
		const char* id = xe->Value();
		eOptionB* o = Find(XmlNameToOptName(id));
		if(!o)
			continue;
		o->Load(xe);
	}
	loading = false;
}
void Done()
{
	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	doc.LinkEndChild(decl);
	TiXmlElement* root = new TiXmlElement("UnrealSpeccyPortable");
	doc.LinkEndChild(root);
	TiXmlElement* owner = new TiXmlElement("Options");
	root->LinkEndChild(owner);
	for(eRootOptionB* r = eRootOptionB::First(); r; r = r->Next())
	{
		r->OptionB()->Store(owner);
	}
	doc.SaveFile(FileName());
}

#else//USE_CONFIG

void Init()
{
	eOA::SortByOrder();
	Apply();
}
void Done() {}

#endif//USE_CONFIG

}
//namespace xOptions
