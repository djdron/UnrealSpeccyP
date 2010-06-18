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

#ifndef	__UI_CONTROL_H__
#define	__UI_CONTROL_H__

#include "ui.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

//*****************************************************************************
//	eControl
//-----------------------------------------------------------------------------
class eControl
{
public:
	eControl() : parent(NULL), changed(true), focused(false), last_focused(false), background(0), id(-1) {}
	virtual ~eControl() {}
	virtual void Init() {}
	byte Id() const { return id; }
	void Id(byte v) { id = v; }
	void Parent(eControl* c) { parent = c; }
	void Focused(bool b) { focused = b; }
	eRect& Bound() { return bound; }
	eRect ScreenBound()
	{
		eRect r = Bound();
		for(eControl* c = parent; c; c = c->parent)
		{
			r.Move(c->Bound().Beg());
		}
		return r;
	}
	xUi::eRGBAColor& Background() { return background; }
	virtual void Update()
	{
		if(changed || (focused != last_focused))
		{
			last_focused = focused;
			if(focused)	DrawRect(ScreenBound(), FOCUS_COLOR, 0x08ffffff);
			else		DrawRect(ScreenBound(), background, 0x08ffffff);
		}
	}
	virtual bool OnKey(char key, dword flags) = 0;
	virtual void Notify(byte n) { OnNotify(n, id); }
protected:
	virtual void OnNotify(byte n, byte from)
	{
		SAFE_CALL(parent)->OnNotify(n, from);
	}
	enum { FOCUS_COLOR = 0x08008000 };
protected:
	eRect bound;
	eControl* parent;
	bool changed;
	bool focused;
	bool last_focused;
	eRGBAColor background;
	byte id;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_CONTROL_H__
