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

#ifndef _HAIKU_WEB_WINDOW_H
#define _HAIKU_WEB_WINDOW_H

#include <stdio.h>

#include <AppKit.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <StorageKit.h>
#include <IconUtils.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>
#include <StringList.h>
#include <Catalog.h>

const uint32 kFileInvocated = 'SINV';
const uint32 kFileDownload = 'FDNL';
const uint32 kFileOpenDowloadFolder = 'ODNL';
const uint32 kGoToParent = 'GOUP';
const uint32 kPathSelect = 'PSEL';
const uint32 kCancelButtonPush = 'BCAN';
const uint32 kOpenButtonPush = 'BOPN';

namespace xPlatform
{

class WebWindow : public BWindow
{
public:
					WebWindow(BRect frame, BWindow *win);
	virtual void 	MessageReceived(BMessage *msg);
	virtual bool 	QuitRequested();
private:
	void 			UpdateList(void);

	BString			fPath;
	BPath			fLastDownloadedPath;

	BMenuBar*		fMenuBar;
	BListView*		fListView;
	BStringView*	fStatusView;
	BScrollView*	fScrollView;
	BButton*		fUpButton;
	BButton*		fCancelButton;
	BButton*		fOpenButton;
	BPopUpMenu*		fPathPopupMenu;
	BMenuField*		fPathMenuField;
	BWindow*		fMainWindow;
	BBitmap*		fFolderIcon;
	BBitmap*		fZipIcon;	
	BBitmap*		fScriptIcon;
	BBitmap*		fRegularIcon;
	BBitmap*		fUpIcon;
};

}

#endif
