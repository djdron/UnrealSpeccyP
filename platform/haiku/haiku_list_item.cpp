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

#include "haiku_list_item.h"

IconListItem::IconListItem(BBitmap *icon,const char *text, uint32 data, int level, bool expanded) :
	BListItem(level, expanded),
	fIcon(icon),
	fLabel(text),
	fExtraDate(data)
{
	BFont font(be_plain_font);
	font_height heighdata;
	font.GetHeight(&heighdata);
	SetHeight(heighdata.ascent + heighdata.descent + 2);
	SetWidth(font.StringWidth(text));
}

void
IconListItem::DrawItem(BView *owner, BRect frame, bool complete)
{
	owner->SetDrawingMode(B_OP_OVER);

	owner->SetHighColor((IsSelected() || complete ) ? ui_color(B_LIST_SELECTED_BACKGROUND_COLOR) : owner->ViewColor());
	owner->FillRect(frame);

	if (fIcon) {
		owner->MovePenTo(fIcon->Bounds().Width(), frame.top);
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->DrawBitmap(fIcon);
	}

	owner->MovePenTo(frame.left + ((fIcon == NULL) ? 0 : fIcon->Bounds().Width() * 2 ) + 8,
		(frame.bottom + frame.top) / 2.0 + 4.0);
	owner->SetHighColor(ui_color(IsSelected() ? B_LIST_SELECTED_ITEM_TEXT_COLOR : B_LIST_ITEM_TEXT_COLOR));
	owner->DrawString(fLabel.String());
} 


IconMenuListItem::IconMenuListItem(BBitmap *icon,const char *label, BMessage *message) :
	BMenuItem(label, message),
	fIcon(icon)
{
}

void
IconMenuListItem::DrawContent(void)
{
	if (fIcon) {
		Menu()->MovePenTo(fIcon->Bounds().Width(), Frame().top);
		Menu()->SetDrawingMode(B_OP_ALPHA);
		Menu()->DrawBitmap(fIcon);
	}

	Menu()->SetDrawingMode(B_OP_OVER);
	Menu()->MovePenTo(Frame().left + ((fIcon == NULL) ? 0 : fIcon->Bounds().Width() * 2 ) + 8,
		(Frame().bottom + Frame().top) / 2.0 + 4.0);
	Menu()->SetHighColor(ui_color(IsSelected() ? B_MENU_SELECTED_ITEM_TEXT_COLOR : B_MENU_ITEM_TEXT_COLOR));
	Menu()->DrawString(Label());
} 
