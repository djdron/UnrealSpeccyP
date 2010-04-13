#ifndef	__CONTROLS_H__
#define	__CONTROLS_H__

#include "render.h"

#pragma once

namespace xUi
{

//*****************************************************************************
//	eControl
//-----------------------------------------------------------------------------
class eControl
{
public:
	eControl() : parent(NULL), changed(true), background(0) {}
	virtual void Init() {}
	eRect& Bound() { return bound; }
	void Parent(eControl* c) { parent = c; }
	eRect ScreenBound()
	{
		eRect r = Bound();
		for(eControl* c = parent; c; c = c->parent)
		{
			r.Move(c->Bound().beg);
		}
		return r;
	}
	xRender::eRGBAColor& Background() { return background; }
	virtual void Update() = 0;
	virtual void OnKey(char key) {}
protected:
	eRect bound;
	eControl* parent;
	bool changed;
	xRender::eRGBAColor background;
};

//*****************************************************************************
//	eDialog
//-----------------------------------------------------------------------------
class eDialog : public eControl
{
	enum { MAX_CHILDS = 32 };
public:
	eDialog() { *childs = NULL; }
	~eDialog()
	{
		for(int i = 0; childs[i]; ++i)
		{
			delete childs[i];
		}
	}
	void Insert(eControl* child)
	{
		for(int i = 0; i < MAX_CHILDS; ++i)
		{
			if(childs[i])
				continue;
			child->Parent(this);
			child->Background() = background;
			child->Init();
			childs[i] = child;
			childs[i + 1] = NULL;
			break;
		}
	}
	virtual void Update()
	{
		if(changed)
		{
			changed = false;
			xRender::DrawRect(bound, background);
		}
		for(int i = 0; childs[i]; ++i)
		{
			childs[i]->Update();
		}
	}
	virtual void OnKey(char key)
	{
		for(int i = 0; childs[i]; ++i)
		{
			childs[i]->OnKey(key);
		}
	}
protected:
	eControl* childs[MAX_CHILDS + 1];
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
	~eList() { Clear(); }
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
	const char* Selected() const { return items[selected]; }
	int Selector() const { return selected; }
	virtual void Update();
	virtual void OnKey(char key);
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

#endif//__CONTROLS_H__
