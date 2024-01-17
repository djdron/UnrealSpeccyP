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

#include "../platform.h"
#include "../io.h"
#include "../../tools/options.h"
#include "../../options_common.h"
#include "../../tools/io_select.h"

#include "haiku_web_window.h"
#include "haiku_list_item.h"

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "USPWebWindow"

namespace xIo
{
	eFileSelect* FileSelectWEB(const char* path);
	const char* FileSelectWEB_Open(const char* name);
}

namespace xPlatform
{

WebWindow::WebWindow(BRect frame, BWindow *win)
: BWindow(frame, B_TRANSLATE("Open file from web" B_UTF8_ELLIPSIS), B_DOCUMENT_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_CLOSE_ON_ESCAPE),
	fMainWindow(win)
{
	BFont font(be_plain_font);
	font_height heighdata;
	font.GetHeight(&heighdata);
	float height = heighdata.ascent + heighdata.descent;

	BMimeType folder("application/x-vnd.Be-directory");
	BMimeType script("text/x-makefile");
	BMimeType regular("application/octet-stream");
	BMimeType zip("application/zip");

	fFolderIcon = new BBitmap(BRect(0, 0, height, height), B_RGBA32);
	fScriptIcon = new BBitmap(BRect(0, 0, height, height), B_RGBA32);
	fRegularIcon = new BBitmap(BRect(0, 0, height, height), B_RGBA32);
	fZipIcon = new BBitmap(BRect(0, 0, height, height), B_RGBA32);
	fUpIcon = new BBitmap(BRect(0, 0, height, height), B_RGBA32);

	folder.GetIcon(fFolderIcon, B_MINI_ICON);
	script.GetIcon(fScriptIcon, B_MINI_ICON);
	regular.GetIcon(fRegularIcon, B_MINI_ICON);
	zip.GetIcon(fZipIcon, B_MINI_ICON);
	
	BResources trackerResource("/system/lib/libtracker.so");
	size_t length = 0;
	const void* data = trackerResource.LoadResource('VICN', 1044, &length);
	BIconUtils::GetVectorIcon((uint8*)data, length, fUpIcon);
	
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &fLastDownloadedPath) == B_OK)
		fLastDownloadedPath.Append("USP");
	else
		fLastDownloadedPath.SetTo("/boot/home/config/settings/USP");
	
	fMenuBar = new BMenuBar("menu_bar", B_ITEMS_IN_ROW, B_INVALIDATE_AFTER_LAYOUT);
	BMenu* menu = new BMenu(B_TRANSLATE("File"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Open"), new BMessage(kFileInvocated), 'O'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Download file"), new BMessage(kFileDownload), 'D'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Open download folder"), new BMessage(kFileOpenDowloadFolder), 'J'));
	fMenuBar->AddItem(menu);
	menu->SetTargetForItems(this);

	fPathPopupMenu = new BPopUpMenu("Root");
	fPathMenuField = new BMenuField("path", "", fPathPopupMenu);
	fPathMenuField->SetDivider(0);

	fListView = new BListView("list view", B_SINGLE_SELECTION_LIST);
	fListView->SetInvocationMessage(new BMessage(kFileInvocated));

	fScrollView = new BScrollView("web scroll", fListView, 0, false, true);

	fUpButton = new BButton("", new BMessage(kGoToParent));
	fUpButton->SetIcon(fUpIcon, 0);
	fUpButton->SetFlat(true);

	fCancelButton = new BButton(B_TRANSLATE("Cancel"), new BMessage(kCancelButtonPush));
	fOpenButton = new BButton(B_TRANSLATE("Open"), new BMessage(kOpenButtonPush));
	fOpenButton->MakeDefault(true);
	
	BFont smallFont(be_plain_font);
	smallFont.SetSize(ceilf(font.Size() * 0.75));
	fStatusView = new BStringView("list status", B_TRANSLATE("items"));
	fStatusView->SetFont(&smallFont, B_FONT_SIZE);
	fStatusView->SetAlignment(B_ALIGN_LEFT);

	fPath.SetTo("");

	const float spacing = be_control_look->DefaultItemSpacing();

	BGroupLayout* statusGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, 0)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fStatusView)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.AddGlue()
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.SetInsets(0, 0, 0, 0))
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
		.SetInsets(1, 0, 1, 0);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.Add(BLayoutBuilder::Group<>(B_VERTICAL, B_USE_DEFAULT_SPACING)
			.AddGroup(B_HORIZONTAL, 0)
				.Add(fUpButton)
				.Add(fPathMenuField)
				.AddGlue()
				.AddGlue()
				.AddGlue()
				.AddGlue()
			.End()
			.AddGroup(B_VERTICAL, 0)
				.Add(fScrollView)
				.Add(statusGroup)
			.End()
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
				.AddGlue()
				.Add(fCancelButton)
				.Add(fOpenButton)
				.AddStrut(B_USE_DEFAULT_SPACING)
			.End()
			.SetInsets(B_USE_WINDOW_SPACING))
		.End();

	SetSizeLimits(512, 2048, 320, 2048);

	AddShortcut(B_UP_ARROW, 0, new BMessage(kGoToParent));
	AddShortcut(B_DOWN_ARROW, 0, new BMessage(kFileInvocated));

	CenterOnScreen();
	UpdateList();
}

static int _StrICmp(const void* item1,	const void* item2)
{
	IconListItem *i1 = *((IconListItem**)item1);
	IconListItem *i2 = *((IconListItem**)item2);
	return strcasecmp(i1->GetLabel(), i2->GetLabel());
}

void
WebWindow::UpdateList(void)
{
	fListView->MakeEmpty();
	fPathPopupMenu->RemoveItems(0, fPathPopupMenu->CountItems(), true);

	BList dirs;
	xIo::eFileSelect* fs = xIo::FileSelectWEB(fPath.String());
	for(; fs->Valid(); fs->Next()) {
		if(fs->IsDir())
			dirs.AddItem(new IconListItem(fFolderIcon, fs->Name(), B_DIRECTORY_NODE, 0, false));
	}

	BList files;
	fs = xIo::FileSelectWEB(fPath.String());
	for(; fs->Valid(); fs->Next()) {
		if(fs->IsFile()) {
			BString fileName(fs->Name());
			BBitmap *icon = fRegularIcon;
			if (fileName.IEndsWith(".zip") || fileName.IEndsWith(".rar"))
				icon = fZipIcon;
			if (fileName.IEndsWith(".rzx"))
				icon = fScriptIcon;
			files.AddItem(new IconListItem(icon, fs->Name(), B_FILE_NODE, 0, false));
		}
	}
	dirs.SortItems(_StrICmp);
	files.SortItems(_StrICmp);
	fListView->AddList(&dirs);
	fListView->AddList(&files);
	
	BStringList pathList;
	fPath.Split("/", true, pathList);
	BString path;
	BMessage *message = new BMessage(kPathSelect);
	message->AddString("path", path);
	BMenuItem *item = new IconMenuListItem(fFolderIcon, "/", message);
	fPathPopupMenu->AddItem(item);
	for (int i = 0; i < pathList.CountStrings(); i++) {
		message = new BMessage(kPathSelect);
		path << pathList.StringAt(i) << "/";
		message->AddString("path", path);
		item = new IconMenuListItem(fFolderIcon, pathList.StringAt(i).String(), message);
		fPathPopupMenu->AddItem(item);
	}
	item->SetMarked(true);

	fUpButton->SetEnabled(pathList.CountStrings() != 0);

	BString statusText;
	statusText << " " << fListView->CountItems() << " " << B_TRANSLATE("items") << " ";
	fStatusView->SetText(statusText);
}

void
WebWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case kCancelButtonPush:
		{
			Hide();
			break;
		}
		case kGoToParent:
		{
			char parent[MAXPATHLEN];
			xIo::GetPathParent(parent, fPath.String());
			if(strlen(parent))
				strcat(parent, "/");
			fPath = parent;
			UpdateList();
			break;
		}
		case kPathSelect:
		{
			fPath = message->FindString("path");
			UpdateList();
			break;
		}
		case kFileOpenDowloadFolder:
		{
			BString path("open ");
			path << "\"" << fLastDownloadedPath.Path() << "\"";
			system(path.String());
			break;
		}
		case kFileDownload:
		case kOpenButtonPush:
		case kFileInvocated:
		{
			int32 index = fListView->CurrentSelection();
			if (index < 0)
				break;
			IconListItem *item = dynamic_cast<IconListItem*>(fListView->ItemAt(index));
			if (item) {
				if (item->GetData() == B_DIRECTORY_NODE) {
					fPath << item->GetLabel() << "/";
					UpdateList();
				}
				if (item->GetData() == B_FILE_NODE) {
					BString file = fPath;
					file << item->GetLabel();
					const char *selected = xIo::FileSelectWEB_Open(file.String());
					if (selected) {
						if (message->what != kFileDownload) {
							Handler()->OnOpenFile(selected);
							Hide();
						}
						BPath file(selected);
						file.GetParent(&fLastDownloadedPath);
					}
					break;
				}
			}
			break;
		}
	default:
		BWindow::MessageReceived(message);
		break;
	}
}

bool
WebWindow::QuitRequested()
{
	Hide();
	return false;
}

}
