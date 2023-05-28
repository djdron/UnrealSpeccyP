/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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

#ifndef _ICON_LIST_ITEM_H
#define _ICON_LIST_ITEM_H

#include <InterfaceKit.h>

class IconListItem : public BListItem
{ 
	public:
						IconListItem(BBitmap *icon, const char *text, uint32 data, int level, bool expanded); 
		virtual 		~IconListItem() { };
		const char*		GetLabel() { return fLabel.String(); }
		const uint32	GetData() { return fExtraDate; }

		virtual void 	DrawItem(BView *owner, BRect frame, bool complete = false); 
		virtual void 	Update(BView *owner, const BFont *font) { };

	private:
		BBitmap*		fIcon;
		BString			fLabel;
		uint32			fExtraDate;
};

class IconMenuListItem : public BMenuItem
{ 
	public:
						IconMenuListItem(BBitmap *icon, const char *label, BMessage* message);
		virtual void	DrawContent();
	private:
		BBitmap*		fIcon;
};


#endif
