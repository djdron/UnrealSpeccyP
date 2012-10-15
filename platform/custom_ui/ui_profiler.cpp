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

#include "../../ui/ui_dialog.h"
#include "../../ui/ui_list.h"
#include "../../tools/profiler.h"

#ifdef USE_UI
#ifdef USE_PROFILER

namespace xUi
{

class eProfiler : public eDialog
{
	typedef eDialog eInherited;
public:
	virtual void Init();
	virtual void Update();
	virtual bool OnKey(char key, dword flags);
protected:
	void SectionsUpdate();
	eList* sections;
	int frames;
};
void eProfiler::Init()
{
	background = COLOR_BACKGROUND;
	int cnt = 0;
	for(xProfiler::eSection* s = xProfiler::eSection::First(); s; s = s->Next())
	{
		++cnt;
	}
	ePoint margin(6, 6);
	ePoint size(200, cnt*FontSize().y);
	sections = new eList;
	Insert(sections);
	eRect r_sec(size);
	r_sec.Move(margin);
	sections->Bound() = r_sec;

	eRect r_dlg(ePoint(size.x + margin.x*2, size.y + margin.y*2));
	r_dlg.Move(ePoint(10, 10));
	Bound() = r_dlg;
	frames = 0;
	SectionsUpdate();
}
void eProfiler::Update()
{
	if(++frames > 10)
	{
		frames = 0;
		SectionsUpdate();
	}
	eInherited::Update();
}
bool eProfiler::OnKey(char key, dword flags)
{
	switch(key)
	{
	case 'e':	xProfiler::eSection::ResetAll(); return true;
	}
	return eInherited::OnKey(key, flags);
}
void eProfiler::SectionsUpdate()
{
	int s = sections->Selected();
	sections->Clear();
	for(xProfiler::eSection* s = xProfiler::eSection::First(); s; s = s->Next())
	{
		sections->Insert(s->Dump());
	}
	sections->Selected(s);
}
eDialog* CreateProfiler() { return new eProfiler; }

}
//namespace xUi

#endif//USE_PROFILER
#endif//USE_UI
