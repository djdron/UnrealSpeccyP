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

#ifndef _KEYBOARD_WINDOW_H
#define _KEYBOARD_WINDOW_H

#include <stdio.h>

#include <AppKit.h>
#include <InterfaceKit.h>
#include <StringList.h>
#include <Catalog.h>

#define ZXB_BUTTON		0
#define ZXB_SEPARATOR	1
#define ZXB_NEWLINE		2
#define ZXB_END			3

const uint32 kHelpKeyboardLayout = 'HKBD';
const uint32 kKeyHighlight = 'HGLT';

typedef struct {
	int32 size;
	int32 type;
	const char *label;
	const char *top;
	const char *bottom;
	int8 udg;
	const char *symbol;
	const char *command;
	uint32 key;
} zxbutton;

class ZXButton : public BButton
{
public:
					ZXButton(BRect rect, zxbutton *button);
	virtual void	Draw(BRect r);
	virtual	void	MouseDown(BPoint where);
	virtual	void	MouseUp(BPoint where);
	void			SetHighlight(bool enable);
	zxbutton*		ButtonData(void) { return fButtonData; }
private:
	zxbutton*		fButtonData;
	bool			fButtonHighlight;
	BFont			fLabelFont;
	BFont			fCommandFont;
	BFont			fExtendFont;
};

class KeyboardWindow : public BWindow
{
public:
					KeyboardWindow(BRect frame, BWindow *win);
	virtual void	MessageReceived(BMessage *msg);
	virtual bool	QuitRequested();

	void			AddButton(zxbutton *button);
private:
	BView*			fView;
	BWindow*		fMainWindow;
	BPoint			fButtonPosition;
	float			fButtonSize;
	float			fButtonSpace;
};
//------------------------------------------------------------------------------
#endif
