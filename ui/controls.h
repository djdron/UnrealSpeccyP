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

#ifndef	__CONTROLS_H__
#define	__CONTROLS_H__

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
	virtual void OnKey(char key, dword flags) = 0;
protected:
	virtual void Notify(byte n, byte from)
	{
		OnNotify(n, from);
		SAFE_CALL(parent)->Notify(n, from);
	}
	virtual void OnNotify(byte n, byte from) {}
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

//*****************************************************************************
//	eDialog
//-----------------------------------------------------------------------------
class eDialog : public eControl
{
	enum { MAX_CHILDS = 64 };
public:
	eDialog() : focused(NULL) { *childs = NULL; }
	virtual ~eDialog()
	{
		for(int i = 0; childs[i]; ++i)
		{
			delete childs[i];
		}
		DrawRect(bound, 0);
	}
	void Insert(eControl* child);
	virtual void Update();
	virtual void OnKey(char key, dword flags);
protected:
	void ChooseFocus(char key);
protected:
	eControl* childs[MAX_CHILDS + 1];
	eControl* focused;
};

//*****************************************************************************
//	eButton
//-----------------------------------------------------------------------------
class eButton : public eControl
{
	enum { MAX_TEXT_SIZE = 64 };
	enum { PUSH_COLOR = 0x080000b0 };
	typedef eControl eInherited;
public:
	eButton() : pushed(false), last_pushed(false) { *text = '\0'; }
	void Text(const char* s) { assert(strlen(s) <= MAX_TEXT_SIZE); strcpy(text, s); }
	void Push(bool b) { pushed = b; }
	virtual void Update();
	virtual void OnKey(char key, dword flags);
	enum eNotify { N_PUSH, N_POP };
protected:
	char text[MAX_TEXT_SIZE + 1];
	bool pushed;
	bool last_pushed;
};

//*****************************************************************************
//	eList
//-----------------------------------------------------------------------------
class eList : public eControl
{
	enum { MAX_ITEMS = 256 };
	enum { CURSOR_COLOR = 0x08b06000 };
public:
	eList() : size(0), last_selected(0), selected(0), page_begin(0), page_size(0) { *items = NULL; }
	virtual ~eList() { Clear(); }
	void Clear()
	{
		changed = true;
		for(int i = 0; items[i]; ++i)
		{
			delete items[i];
		}
		*items = NULL;
		size = last_selected = selected = page_begin = page_size = 0;
	}
	void Insert(const char* item);
	const char* Selected() const { return size ? items[selected] : NULL; }
	int Selector() const { return size ? selected : -1; }
	virtual void Update();
	virtual void OnKey(char key, dword flags);
protected:
	const char* items[MAX_ITEMS + 1];
	int size;
	int last_selected;
	int selected;
	int page_begin;
	int page_size;
};

}
//namespace xUi

#endif//USE_UI

#endif//__CONTROLS_H__
