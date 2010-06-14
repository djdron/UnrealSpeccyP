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
#include "tinyxml/tinyxml.h"
#include "../platform/io.h"

namespace xOptions
{

static const char* FileName() { return xIo::ProfilePath(".options.xml"); }
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
//=============================================================================
//	Load
//-----------------------------------------------------------------------------
void Load()
{
	TiXmlDocument doc;
	if(!doc.LoadFile(FileName()))
		return;

	TiXmlElement* root = doc.RootElement();
	if(!root)
		return;

	TiXmlElement* opts = root->FirstChild("Options")->FirstChildElement();
	if(!opts)
		return;

	for(; opts; opts = opts->NextSiblingElement())
	{
		eOptionB* o = eOptionB::Find(XmlNameToOptName(opts->Value()));
		if(!o)
			continue;
		const char* v = opts->GetText();
		o->Value(v ? v : "");
	}

	for(eOptionB* o = eOptionB::First(); o; o = o->Next())
	{
		o->Apply();
	}
}
//=============================================================================
//	Store
//-----------------------------------------------------------------------------
void Store()
{
	TiXmlDocument doc;
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);

	TiXmlElement* root = new TiXmlElement("UnrealSpeccyPortable");
	doc.LinkEndChild(root);
	TiXmlElement* opts = new TiXmlElement("Options");
	root->LinkEndChild(opts);

	for(eOptionB* o = eOptionB::First(); o; o = o->Next())
	{
		if(!o->Storeable())
			continue;
		TiXmlElement* msg;
		msg = new TiXmlElement(OptNameToXmlName(o->Name()));
		msg->LinkEndChild(new TiXmlText(o->Value()));
		opts->LinkEndChild(msg);
	}
	doc.SaveFile(FileName());
}

}
//namespace xOptions
