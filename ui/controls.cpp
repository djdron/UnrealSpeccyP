#include "../std.h"
#include "controls.h"

namespace xUi
{

//=============================================================================
//	eList::Insert
//-----------------------------------------------------------------------------
void eList::Insert(const char* item)
{
	changed = true;
	for(int i = 0; i < MAX_ITEMS; ++i)
	{
		if(items[i])
			continue;
		char* s = new char[strlen(item) + 1];
		memcpy(s, item, strlen(item) + 1);
		items[i] = s;
		items[i + 1] = NULL;
		size = i + 1;
		break;
	}
}
//=============================================================================
//	eList::Update
//-----------------------------------------------------------------------------
void eList::Update()
{
	eRect sr = ScreenBound();
	if(changed)
	{
		xRender::DrawRect(sr, background);
		eRect r(sr.left, sr.top, sr.right, sr.top + xRender::FontHeight());
		int i = page_begin;
		for(; items[i]; ++i)
		{
			if(r.bottom > sr.bottom)
				break;
			xRender::DrawText(r, items[i]);
			r.Move(ePoint(0, xRender::FontHeight()));
		}
		page_size = i - page_begin;
	}
	if((changed || (selected != last_selected)) && page_size)
	{
		eRect cursor(sr.left, 0, sr.right, 0);
		cursor.top = sr.top + (last_selected - page_begin) * xRender::FontHeight();
		cursor.bottom = cursor.top + xRender::FontHeight();
		xRender::DrawRect(cursor, background, 0x08ffffff);
		last_selected = selected;
		if(selected < page_begin)
		{
			page_begin = selected;
			changed = true;
			return;
		}
		if(selected >= page_begin + page_size)
		{
			page_begin = selected - page_size + 1;
			changed = true;
			return;
		}
		cursor.top = sr.top + (selected - page_begin) * xRender::FontHeight();
		cursor.bottom = cursor.top + xRender::FontHeight();
		xRender::DrawRect(cursor, CURSOR_COLOR, 0x08ffffff);
	}
	changed = false;
}
//=============================================================================
//	eList::OnKey
//-----------------------------------------------------------------------------
void eList::OnKey(char key)
{
	switch(key)
	{
	case 'Q': --selected; break;
	case 'A': ++selected; break;
	}
	if(selected < 0) selected = size - 1;
	if(selected >= size) selected = 0;
}

}
//namespace xUi
