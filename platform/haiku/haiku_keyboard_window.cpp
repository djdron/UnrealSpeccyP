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

#include "haiku_keyboard_window.h"

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "USPKeyboardWindow"

static zxbutton zxkeymap[] = {
	{100,	ZXB_BUTTON,		"BREAK",		NULL,		NULL,		-1,	NULL,	NULL,	0x01 },
	{100,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"ABOUT",		"F1",		NULL,		-1,	NULL,	NULL,	0x02 },
	{100,	ZXB_BUTTON,		"SAVE",			"F2",		NULL,		-1,	NULL,	NULL,	0x03 },
	{100,	ZXB_BUTTON,		"OPEN",			"F3",		NULL,		-1,	NULL,	NULL,	0x04 },
	{100,	ZXB_BUTTON,		"WEB\nOPEN",	"F4",		NULL,		-1,	NULL,	NULL,	0x05 },
	{25,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"TAPE",			"F5",		NULL,		-1,	NULL,	NULL,	0x06 },
	{100,	ZXB_BUTTON,		"QUICK\nSAVE",	"F6",		NULL,		-1,	NULL,	NULL,	0x07 },
	{100,	ZXB_BUTTON,		"QUICK\nLOAD",	"F7",		NULL,		-1,	NULL,	NULL,	0x08 },
	{100,	ZXB_BUTTON,		"VIRT.\nKEYB.",	"F8",		NULL,		-1,	NULL,	NULL,	0x09 },
	{25,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"OSD\nKEYB.",	"F9",		NULL,		-1,	NULL,	NULL,	0x0A },
	{100,	ZXB_BUTTON,		"OSD\nMENU",	"F10",		NULL,		-1,	NULL,	NULL,	0x0B },
	{100,	ZXB_BUTTON,		"48K\n128K",	"F11",		NULL,		-1,	NULL,	NULL,	0x0C },
	{100,	ZXB_BUTTON,		"RESET",		"F12",		NULL,		-1,	NULL,	NULL,	0x0D },
	{25,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x0E },
	{100,	ZXB_BUTTON,		NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x0F },
	{100,	ZXB_BUTTON,		"PAUSE",		NULL,		NULL,		-1,	NULL,	NULL,	0x10 },
	{140,	ZXB_NEWLINE,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"`",			NULL,		NULL,		-1,	NULL,	NULL,	0x11 },
	{100,	ZXB_BUTTON,		"1",			"EDIT",		"DEF FN",	2,	"!",	NULL,	0x12 },
	{100,	ZXB_BUTTON,		"2",			"C.LOCK",	"FN",		1,	"@",	NULL,	0x13 },
	{100,	ZXB_BUTTON,		"3",			"TR.VIDEO",	"LINE",		3,	"#",	NULL,	0x14 },
	{100,	ZXB_BUTTON,		"4",			"IN.VIDEO",	"OPEN#",	8,	"$",	NULL,	0x15 },
	{100,	ZXB_BUTTON,		"5",			"←",		"CLOSE#",	10,	"%",	NULL,	0x16 },
	{100,	ZXB_BUTTON,		"6",			"↓",		"MOVE",		9,	"&",	NULL,	0x17 },
	{100,	ZXB_BUTTON,		"7",			"↑",		"ERASE",	11,	"`",	NULL,	0x18 },
	{100,	ZXB_BUTTON,		"8",			"→",		"POINT",	0,	"(",	NULL,	0x19 },
	{100,	ZXB_BUTTON,		"9",			"GRAPHICS",	"CAT",		-1,	")",	NULL,	0x1A },
	{100,	ZXB_BUTTON,		"0",			"DELETE",	"FORMAT",	-1,	"_",	NULL,	0x1B },
	{100,	ZXB_BUTTON,		"_\n-",			NULL,		NULL,		-1,	NULL,	NULL,	0x1C },
	{100,	ZXB_BUTTON,		"+\n=",			NULL,		NULL,		-1,	NULL,	NULL,	0x1D },
	{150,	ZXB_BUTTON,		"DELETE",		NULL,		NULL,		-1,	NULL,	NULL,	0x1E },
	{25,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"EDIT",			NULL,		NULL,		-1,	NULL,	NULL,	0x1F },
	{100,	ZXB_BUTTON,		"TRUE\nVIDEO",	NULL,		NULL,		-1,	NULL,	NULL,	0x20 },
	{100,	ZXB_BUTTON,		"VOL+",			NULL,		NULL,		-1,	NULL,	NULL,	0x21 },
	{100,	ZXB_NEWLINE,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{150,	ZXB_BUTTON,		"EXTENDED",		NULL,		NULL,		-1,	NULL,	NULL,	0x26 },
	{100,	ZXB_BUTTON,		"Q",			"SIN",		"ASN",		-1,	"<=",	"PLOT",	0x27 },
	{100,	ZXB_BUTTON,		"W",			"COS",		"ACS",		-1,	"<>",	"DRAW",	0x28 },
	{100,	ZXB_BUTTON,		"E",			"TAN",		"ATN",		-1,	">=",	"REM",	0x29 },
	{100,	ZXB_BUTTON,		"R",			"INT",		"VERIFY",	-1,	"<",	"RUN",	0x2A },
	{100,	ZXB_BUTTON,		"T",			"RND",		"MERGE",	-1,	">",	"RAND",	0x2B },
	{100,	ZXB_BUTTON,		"Y",			"STR$",		"[",		-1,	"AND",	"RETURN",0x2C},
	{100,	ZXB_BUTTON,		"U",			"CHR$",		"]",		-1,	"OR",	"IF",	0x2D },
	{100,	ZXB_BUTTON,		"I",			"CODE",		"IN",		-1,	"AT",	"INPUT",0x2E },
	{100,	ZXB_BUTTON,		"O",			"PEEK",		"OUT",		-1,	";",	"POKE",	0x2F },
	{100,	ZXB_BUTTON,		"P",			"TAB",		"©",		-1,	"\"",	"PRINT",0x30 },
	{100,	ZXB_BUTTON,		"{\n[",			NULL,		NULL,		-1,	NULL,	NULL,	0x31 },
	{100,	ZXB_BUTTON,		"}\n]",			NULL,		NULL,		-1,	NULL,	NULL,	0x32 },
	{100,	ZXB_BUTTON,		"\\",			NULL,		NULL,		-1,	NULL,	NULL,	0x33 },
	{25,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"DELETE",		NULL,		NULL,		-1,	NULL,	NULL,	0x34 },
	{100,	ZXB_BUTTON,		"INV.\nVIDEO",	NULL,		NULL,		-1,	NULL,	NULL,	0x35 },
	{100,	ZXB_BUTTON,		"VOL-",			NULL,		NULL,		-1,	NULL,	NULL,	0x36 },
	{100,	ZXB_NEWLINE,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{180,	ZXB_BUTTON,		"CAPS LOCK",	NULL,		NULL,		-1,	NULL,	NULL,	0x3B },
	{100,	ZXB_BUTTON,		"A",			"READ",		"~",		-1,	"STOP",	"NEW",	0x3C },
	{100,	ZXB_BUTTON,		"S",			"RESTORE",	"|",		-1,	"NOT",	"SAVE",	0x3D },
	{100,	ZXB_BUTTON,		"D",			"DATA",		"\\",		-1,	"STEP",	"DIM",	0x3E },
	{100,	ZXB_BUTTON,		"F",			"SGN",		"{",		-1,	"TO",	"FOR",	0x3F },
	{100,	ZXB_BUTTON,		"G",			"ABS",		"}",		-1,	"THEN",	"GOTO",	0x40 },
	{100,	ZXB_BUTTON,		"H",			"SQR",		"CIRCLE",	-1,	"^",	"GOSUB",0x41 },
	{100,	ZXB_BUTTON,		"J",			"VAL",		"VAL$",		-1,	"-",	"LOAD",	0x42 },
	{100,	ZXB_BUTTON,		"K",			"LEN",		"SCREEN$",	-1,	"+",	"LIST",	0x43 },
	{100,	ZXB_BUTTON,		"L",			"USR",		"ATTR",		-1,	"=",	"LET",	0x44 },
	{100,	ZXB_BUTTON,		":\n;",			NULL,		NULL,		-1,	NULL,	NULL,	0x45 },
	{100,	ZXB_BUTTON,		"\"\n'",		NULL,		NULL,		-1,	NULL,	NULL,	0x46 },
	{170,	ZXB_BUTTON,		"ENTER",		NULL,		NULL,		-1,	NULL,	NULL,	0x47 },
	{100,	ZXB_NEWLINE,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{225,	ZXB_BUTTON,		"CAPS SHIFT",	NULL,		NULL,		-1,	NULL,	NULL,	0x4B },
	{100,	ZXB_BUTTON,		"Z",			"LN",		"BEEP",		-1,	":",	"COPY",	0x4C },
	{100,	ZXB_BUTTON,		"X",			"EXP",		"INK",		-1,	"₤",	"CLEAR",0x4D },
	{100,	ZXB_BUTTON,		"C",			"LPRINT",	"PAPER",	-1,	"?",	"COUNT",0x4E },
	{100,	ZXB_BUTTON,		"V",			"LLIST",	"FLASH",	-1,	"/",	"CLS",	0x4F },
	{100,	ZXB_BUTTON,		"B",			"BIN",		"BRIGHT",	-1,	"*",	"BORDER",0x50},
	{100,	ZXB_BUTTON,		"N",			"INKEY$",	"OVER",		-1,	",",	"NEXT",	0x51 },
	{100,	ZXB_BUTTON,		"M",			"PI",		"INVERSE",	-1,	".",	"PAUSE",0x52 },
	{100,	ZXB_BUTTON,		"<\n,",			NULL,		NULL,		-1,	NULL,	NULL,	0x53 },
	{100,	ZXB_BUTTON,		">\n.",			NULL,		NULL,		-1,	NULL,	NULL,	0x54 },
	{100,	ZXB_BUTTON,		"?\n/",			NULL,		NULL,		-1,	NULL,	NULL,	0x55 },
	{225,	ZXB_BUTTON,		"CAPS SHIFT",	NULL,		NULL,		-1,	NULL,	NULL,	0x56 },
	{125,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"↑",			NULL,		NULL,		-1,	NULL,	NULL,	0x57 },	
	{100,	ZXB_NEWLINE,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{125,	ZXB_BUTTON,		"SYMBOL\nSHIFT",NULL,		NULL,		-1,	NULL,	NULL,	0x5C },
	{125,	ZXB_BUTTON,		"GRAPH.",		NULL,		NULL,		-1,	NULL,	NULL,	0x66 },
	{125,	ZXB_BUTTON,		"ALT",			NULL,		NULL,		-1,	NULL,	NULL,	0x5D },
	{575,	ZXB_BUTTON,		"BREAK SPACE",	NULL,		NULL,		-1,	NULL,	NULL,	0x5E },
	{125,	ZXB_BUTTON,		"ALT",			NULL,		NULL,		-1,	NULL,	NULL,	0x5F },
	{125,	ZXB_BUTTON,		"GRAPH.",		NULL,		NULL,		-1,	NULL,	NULL,	0x67 },
	{125,	ZXB_BUTTON,		"OSD\nMENU",	NULL,		NULL,		-1,	NULL,	NULL,	0x68 },
	{125,	ZXB_BUTTON,		"SYMBOL\nSHIFT",NULL,		NULL,		-1,	NULL,	NULL,	0x60 },
	{25,	ZXB_SEPARATOR,	NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
	{100,	ZXB_BUTTON,		"←",			NULL,		NULL,		-1,	NULL,	NULL,	0x61 },
	{100,	ZXB_BUTTON,		"↓",			NULL,		NULL,		-1,	NULL,	NULL,	0x62 },
	{100,	ZXB_BUTTON,		"→",			NULL,		NULL,		-1,	NULL,	NULL,	0x63 },
	{0,		ZXB_END,		NULL,			NULL,		NULL,		-1,	NULL,	NULL,	0x00 },
};

ZXButton::ZXButton(BRect rect, zxbutton *button)
: BButton(rect, button->label, NULL, NULL, 0, 0),
	fButtonData(button),
	fButtonHighlight(false)
{
	BFont font;
	GetFont(&font);
	fLabelFont = font;
	fCommandFont = font;
	fExtendFont = font;

	fLabelFont.SetSize(11);
	fLabelFont.SetFamilyAndFace("Noto Sans", B_BOLD_FACE);

	fCommandFont.SetSize(8);
	fCommandFont.SetFamilyAndFace("Noto Sans", B_REGULAR_FACE);

	fExtendFont.SetSize(8);
	fExtendFont.SetFamilyAndFace("Noto Sans", B_REGULAR_FACE);
}

void
ZXButton::SetHighlight(bool enable)
{
	fButtonHighlight = enable;
	Invalidate();
}

void 
ZXButton::Draw(BRect rect)
{
	BButton::Draw(rect);

	bool simpleButton = fButtonData->label &&
		!fButtonData->bottom &&
		!fButtonData->command &&
		!fButtonData->symbol;

	if (fButtonHighlight) {
		SetDrawingMode(B_OP_ALPHA);
		rgb_color color = ui_color(B_CONTROL_HIGHLIGHT_COLOR);
		color.alpha = 80;
		SetHighColor(color);
		FillRect(Bounds());
		SetDrawingMode(B_OP_COPY);
	}

	if (fButtonData->label) {
		if (simpleButton) {
			SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
			SetFont(&fLabelFont);
			font_height heighdata;
			fLabelFont.GetHeight(&heighdata);
			BStringList lines;
			BString label(fButtonData->label);
			label.Split("\n", false, lines);
			float height = heighdata.ascent + heighdata.descent - 3;
			float totalHeight = lines.CountStrings() * height;		
			for (int i = 0; i < lines.CountStrings(); i++) {
				float width = fLabelFont.StringWidth(lines.StringAt(i).String());
				BPoint pos((Bounds().Width() - width) / 2,
					(Bounds().Height() - totalHeight) / 2 + heighdata.ascent + (height * i));
				DrawString(lines.StringAt(i).String(), pos);
			}
		} else {
			SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
			SetFont(&fLabelFont);
			font_height heighdata;
			fLabelFont.GetHeight(&heighdata);
			float height = heighdata.ascent + heighdata.descent;
			BPoint pos(Bounds().left + 4, ((Bounds().Height() - height) / 2) + heighdata.ascent);
			DrawString(fButtonData->label, pos);
		}
	}

	if (fButtonData->top) {
		SetHighColor(0, 100, 0);
		SetFont(&fExtendFont);
		font_height heighdata;
		fExtendFont.GetHeight(&heighdata);
		float height = heighdata.ascent + heighdata.descent;
		float width = fExtendFont.StringWidth(fButtonData->top);
		BPoint pos((Bounds().Width() - width) / 2, Bounds().top + heighdata.ascent + 3);
		DrawString(fButtonData->top, pos);
	}

	if (fButtonData->bottom) {
		SetHighColor(0, 0, 100);
		SetFont(&fExtendFont);
		font_height heighdata;
		fExtendFont.GetHeight(&heighdata);
		float height = heighdata.ascent + heighdata.descent;
		float width = fExtendFont.StringWidth(fButtonData->bottom);
		BPoint pos((Bounds().Width() - width) / 2, Bounds().bottom - (heighdata.descent + 2));
		DrawString(fButtonData->bottom, pos);
	}

	if (fButtonData->command) {
		SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
		SetFont(&fCommandFont);
		font_height heighdata;
		fCommandFont.GetHeight(&heighdata);
		float height = heighdata.ascent + heighdata.descent;
		float width = fCommandFont.StringWidth(fButtonData->command);
		BPoint pos(Bounds().Width() - (width + 3), ((Bounds().Height() - height) / 2) + height + 4);
		DrawString(fButtonData->command, pos);
	}

	if (fButtonData->symbol) {
		SetHighColor(100, 0, 0);
		SetFont(&fCommandFont);
		font_height heighdata;
		fCommandFont.GetHeight(&heighdata);
		float height = heighdata.ascent + heighdata.descent;
		float width = fCommandFont.StringWidth(fButtonData->symbol);
		BPoint pos(Bounds().Width() - (width + 3), ((Bounds().Height() - height) / 2) + heighdata.ascent - (heighdata.descent + 2));
		DrawString(fButtonData->symbol, pos);
	}

	if (fButtonData->udg >= 0) {
		SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
		SetFont(&fLabelFont);
		font_height heighdata;
		fLabelFont.GetHeight(&heighdata);
		int size = ((heighdata.ascent + heighdata.descent) / 2) - 3;
		BPoint point(Bounds().Width() / 2, Bounds().Height() /2);
		StrokeRect(BRect(point.x - size, point.y - size, point.x + size, point.y + size));
		if (fButtonData->udg & 0x01)
			FillRect(BRect(point.x - size, point.y - size, point.x, point.y));
		if (fButtonData->udg & 0x02)
			FillRect(BRect(point.x, point.y - size, point.x + size, point.y));
		if (fButtonData->udg & 0x04)
			FillRect(BRect(point.x - size, point.y, point.x, point.y + size));
		if (fButtonData->udg & 0x08)
			FillRect(BRect(point.x, point.y, point.x + size, point.y + size));			
	}
}

void
ZXButton::MouseDown(BPoint where)
{
	BMessage *message = new BMessage(B_KEY_DOWN);
	message->AddInt32("key", fButtonData->key);
	Window()->PostMessage(message);
	BButton::MouseDown(where);
}

void
ZXButton::MouseUp(BPoint where)
{
	BMessage *message = new BMessage(B_KEY_UP);
	message->AddInt32("key", fButtonData->key);
	Window()->PostMessage(message);
	BButton::MouseUp(where);
}

KeyboardWindow::KeyboardWindow(BRect frame, BWindow *win)
: BWindow(frame, B_TRANSLATE("Keyboard layout"), B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
	B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_WILL_ACCEPT_FIRST_CLICK),
	fButtonSize(48),
	fButtonSpace(1),
	fMainWindow(win)
{
	fView = new BView(Bounds(), "view", B_FOLLOW_ALL, B_WILL_DRAW);
	fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(fView);
	fButtonPosition = BPoint(10, 10);
	int buttonIndex = 0;
	while (zxkeymap[buttonIndex].type != ZXB_END)
		AddButton(&zxkeymap[buttonIndex++]);

	fButtonPosition.x += 10;
	fButtonPosition.y += fButtonSize + 10;
	ResizeTo(fButtonPosition.x, fButtonPosition.y);
}

void
KeyboardWindow::AddButton(zxbutton *buttonData)
{
	if (buttonData->type == ZXB_BUTTON ) {
		BRect rect(fButtonPosition.x,
			fButtonPosition.y,
			fButtonPosition.x + (fButtonSize * buttonData->size) / 100,
			fButtonPosition.y + fButtonSize);
		rect.InsetBy(fButtonSpace, fButtonSpace);
		ZXButton *button = new ZXButton(rect, buttonData);
		button->SetTarget(this);
		if (!buttonData->label)
			button->SetEnabled(false);
		fView->AddChild(button);
		fButtonPosition.x += (fButtonSize * buttonData->size) / 100;
	} else if (buttonData->type == ZXB_SEPARATOR ) {
		fButtonPosition.x += (fButtonSize * buttonData->size) / 100;
	} else if (buttonData->type == ZXB_NEWLINE ) {
		fButtonPosition.x = 10;
		fButtonPosition.y += (fButtonSize * buttonData->size) / 100;
	}
}

void
KeyboardWindow::MessageReceived(BMessage *msg)
{
	switch(msg->what) {
		case kKeyHighlight:
		{
			uint32 key = msg->FindInt32("key");
			bool highlight = msg->FindBool("highlight");
			for (int i = 0; i < fView->CountChildren(); i++) {
				ZXButton *button = dynamic_cast<ZXButton*>(fView->ChildAt(i));
				if (button) {
					if (button->ButtonData()->key != 0 &&
						button->ButtonData()->key == key) {
						button->SetHighlight(highlight);
						break;
					}
				}
			}
			break;
		}
		case B_UNMAPPED_KEY_UP:
		case B_KEY_UP:
		case B_UNMAPPED_KEY_DOWN:
		case B_KEY_DOWN:
		{
			fMainWindow->PostMessage(msg);
			break;
		}
	default:
		BWindow::MessageReceived(msg);
		break;
	}
}

bool
KeyboardWindow::QuitRequested()
{
	fMainWindow->PostMessage(kHelpKeyboardLayout);
	return false;
}
