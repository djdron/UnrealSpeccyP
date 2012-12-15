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

#ifndef	__UI_TREE_H__
#define	__UI_TREE_H__

#include "ui_control.h"

#pragma once

#ifdef USE_UI

namespace xUi
{

class eTree : public eControl
{
	enum { ITEMS_COUNT = 256 };
	enum eNotify { N_SELECTED };
public:
	eTree() : sub_level(0), size(0), last_selected(-1), selected(-1), page_begin(0), page_size(0) { *items = NULL; }
	virtual ~eTree() { Clear(); }
	void Clear(bool items_only = false);
	void Insert(const char* id, const char* value);
	void SubLevelBegin() { ++sub_level; }
	void SubLevelEnd() { --sub_level; }
	int	Selected() const { return selected; }
	void Selected(int s);
	virtual void Update();
	virtual bool OnKey(char key, dword flags);

	struct eItemInserter
	{
		eItemInserter(eTree& _tree, const char* _id, const char* _value) : tree(_tree)
		{
			tree.Insert(_id, _value);
			tree.SubLevelBegin();
		}
		~eItemInserter() { tree.SubLevelEnd(); }
		eTree& tree;
	};
protected:
	struct eItem
	{
		const char* id;
		const char* value;
		int sub_level;
	};
	eItem* items[ITEMS_COUNT + 1];
	int sub_level;
	int size;
	int last_selected;
	int selected;
	int page_begin;
	int page_size;
};

}
//namespace xUi

#endif//USE_UI

#endif//__UI_TREE_H__
