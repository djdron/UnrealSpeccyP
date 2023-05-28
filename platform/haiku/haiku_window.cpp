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
#include "../../tools/options.h"
#include "../../options_common.h"
#include "../../ui/ui.h"

#include "haiku_app.h"
#include "haiku_window.h"

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "USPWindow"

namespace xPlatform
{

bool CheckJoystick();
void VolumeAudio(float volume);

static struct eOptionWindowState : public xOptions::eOptionString
{
	eOptionWindowState() { customizable = false; }
	virtual const char* Name() const { return "window state"; }
	const char* FormatStr() const { return "position(%d, %d); client_size(%d, %d)"; }
} op_window_state;

static struct eOptionKeyboardWindowState : public xOptions::eOptionString
{
	eOptionKeyboardWindowState() { customizable = false; }
	virtual const char* Name() const { return "keyboard window state"; }
	const char* FormatStr() const { return "position(%d, %d)"; }
} op_keyboard_window_state;

static struct eOptionFullScreen : public xOptions::eOptionBool
{
	eOptionFullScreen() { customizable = false; }
	virtual const char* Name() const { return "full screen"; }
} op_full_screen;

static struct eOptionBilinear : public xOptions::eOptionBool
{
	eOptionBilinear() { customizable = false; }
	virtual const char* Name() const { return "scale bilinear"; }
} op_scale_bilinear;

static struct eOptionXBRFiltering : public xOptions::eOptionBool
{
	eOptionXBRFiltering() { customizable = false; }
	virtual const char* Name() const { return "xbr scale"; }
} op_xbr_scale;

static struct eOptionVirtualKeyboard : public xOptions::eOptionBool
{
	eOptionVirtualKeyboard() { customizable = false; }
	virtual const char* Name() const { return "virtual keyboard"; }
} op_virtual_keyboard;

static struct eOptionSmartBorderDisabled : public xOptions::eOptionBool
{
	eOptionSmartBorderDisabled() { customizable = false; }
	virtual const char* Name() const { return "smart border disabled"; }
} op_smart_border_disabled;

static bool UpdateBoolOption(BMenuItem *item, const char* name, bool toggle)
{
	xOptions::eOption<bool>* op = xOptions::eOption<bool>::Find(name);
	if(op && toggle)
		op->Change();
	bool on = op && *op;
	item->SetMarked(on);
	return on;
}

HaikuPlatformWindow::HaikuPlatformWindow(BRect frame, int w, int h, const char* title, window_type type, uint32 flags)
	: BWindow(frame, title, type, flags),
	fScaleFactor(-1),
	fOpenPanel(NULL),
	fSavePanel(NULL),
	fSaveScreenPanel(NULL),
	fFullscreen(false),
	fSkipScale(false),
	fFiltering(false),
	fXBRFiltering(false),
	fKempsonMouseGrab(false),
	fKeyboardWindow(NULL),
	fWebWindow(NULL),
	fLastPressedKey(0),
	fStateInfoTimer(0)
{
	fMenuBar = new BMenuBar("menu_bar", B_ITEMS_IN_ROW, B_INVALIDATE_AFTER_LAYOUT);
	BMenu* menu = new BMenu(B_TRANSLATE("File"));
	BMenuItem *item = new BMenuItem(BRecentFilesList::NewFileListMenu(B_TRANSLATE("Open" B_UTF8_ELLIPSIS),
		NULL, NULL, this, 9, true, NULL, APP_SIGNATURE),
		new BMessage(kFileOpen));
	item->SetShortcut('O', 0);
	menu->AddItem(item);	
	menu->AddItem(new BMenuItem(B_TRANSLATE("Open from web" B_UTF8_ELLIPSIS), new BMessage(kFileOpenWeb)));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Save" B_UTF8_ELLIPSIS), new BMessage(kFileSave), 'S'));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Save screenshot" B_UTF8_ELLIPSIS), new BMessage(kFileSaveScreen)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Quick load"), new BMessage(kFileQuickOpen), 'O', B_SHIFT_KEY));
	fQuickSaveMenuItem = new BMenuItem(B_TRANSLATE("Quick save"), new BMessage(kFileQuickSave), 'S', B_SHIFT_KEY);
	fQuickSaveMenuItem->SetEnabled(false);
	menu->AddItem(fQuickSaveMenuItem);
	fQuickSaveSlotMenu = new BMenu(B_TRANSLATE("Save slot"));
	for (int slot = 1; slot < 10; slot++) {
		BMessage *message =  new BMessage(kFileQuickSaveSlot);
		message->AddInt32("slot", slot);
		BString title;
		title << slot;
		fQuickSaveSlotMenu->AddItem(new BMenuItem(title, message));
	}
	fQuickSaveSlotMenu->SetRadioMode(true);
	menu->AddItem(fQuickSaveSlotMenu);
	menu->AddSeparatorItem();
	fAutoPlayMenuItem = new BMenuItem(B_TRANSLATE("Auto launch programs"), new BMessage(kFileAutoPlay));
	menu->AddItem(fAutoPlayMenuItem);
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(menu);
	menu = new BMenu(B_TRANSLATE("View"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("100% scale"), new BMessage(kViewScale1x), '1'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("200% scale"), new BMessage(kViewScale2x), '2'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("300% scale"), new BMessage(kViewScale3x), '3'));
	menu->AddSeparatorItem();
	fFilteringMenu = new BMenu(B_TRANSLATE("Filtering"));
	fFilteringMenuItem = new BMenuItem(B_TRANSLATE("Bilinear"), new BMessage(kViewFiltering));
	fFilteringMenu->AddItem(fFilteringMenuItem);
	fXBRFilteringMenuItem = new BMenuItem(B_TRANSLATE("XBR"), new BMessage(kViewXBRFiltering));
	fFilteringMenu->AddItem(fXBRFilteringMenuItem);
	menu->AddItem(fFilteringMenu);
	fSmartBorderScaleMenuItem = new BMenuItem(B_TRANSLATE("Smart border scale"), new BMessage(kViewSmartBorder));
	menu->AddItem(fSmartBorderScaleMenuItem);
	menu->AddSeparatorItem();
	fVirtualKeyboardMenuItem = new BMenuItem(B_TRANSLATE("Virtual keyboard"), new BMessage(kHelpKeyboardLayout));
	menu->AddItem(fVirtualKeyboardMenuItem);
	menu->AddItem(new BMenuItem(B_TRANSLATE("OSD keyboard"), new BMessage(kViewOnScreenKeyboard), 'K'));
	menu->AddItem(new BMenuItem(B_TRANSLATE("OSD menu"), new BMessage(kViewOnScreenMenu)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Full screen"), new BMessage(kViewFullScreen), B_ENTER));
	fMenuBar->AddItem(menu);
	menu = new BMenu(B_TRANSLATE("Device"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Start / Stop tape"), new BMessage(kDeviceStartStopTape)));
	fFastTapeMenuItem = new BMenuItem(B_TRANSLATE("Tape fast"), new BMessage(kDeviceFastTape));
	menu->AddItem(fFastTapeMenuItem);
	menu->AddSeparatorItem();
	fBetaDiskMenu = new BMenu(B_TRANSLATE("Beta disk drive"));
	fBetaDiskMenu->AddItem(new BMenuItem("A", new BMessage(kDeviceBetaDiskA)));
	fBetaDiskMenu->AddItem(new BMenuItem("B", new BMessage(kDeviceBetaDiskB)));
	fBetaDiskMenu->AddItem(new BMenuItem("C", new BMessage(kDeviceBetaDiskC)));
	fBetaDiskMenu->AddItem(new BMenuItem("D", new BMessage(kDeviceBetaDiskD)));
	menu->AddItem(fBetaDiskMenu);
	menu->AddSeparatorItem();
	fSoundChipMenu = new BMenu(B_TRANSLATE("Sound chip"));
	fSoundChipMenu->AddItem(new BMenuItem(B_TRANSLATE("AY-3-8910"), new BMessage(kDeviceSoundChipAY)));
	fSoundChipMenu->AddItem(new BMenuItem(B_TRANSLATE("YM2149F"), new BMessage(kDeviceSoundChipYM)));
	menu->AddItem(fSoundChipMenu);
	fAYStereoMenu = new BMenu(B_TRANSLATE("AY Stereo"));
	fAYStereoMenu->AddItem(new BMenuItem("ABC", new BMessage(kDeviceSoundChipStereoABC)));
	fAYStereoMenu->AddItem(new BMenuItem("ACB", new BMessage(kDeviceSoundChipStereoACB)));
	fAYStereoMenu->AddItem(new BMenuItem("BAC", new BMessage(kDeviceSoundChipStereoBAC)));
	fAYStereoMenu->AddItem(new BMenuItem("BCA", new BMessage(kDeviceSoundChipStereoBCA)));
	fAYStereoMenu->AddItem(new BMenuItem("CAB", new BMessage(kDeviceSoundChipStereoCAB)));
	fAYStereoMenu->AddItem(new BMenuItem("CBA", new BMessage(kDeviceSoundChipStereoCBA)));
	fAYStereoMenu->AddItem(new BMenuItem(B_TRANSLATE("Mono"), new BMessage(kDeviceSoundChipStereoMono)));
	fAYStereoMenu->SetRadioMode(true);
	menu->AddItem(fAYStereoMenu);
	fSoundVolumeMenu = new BMenu(B_TRANSLATE("Volume"));
	for (int volume = 0; volume <= 10; volume++) {
		BMessage *message =  new BMessage(kDeviceSoundVolume);
		message->AddInt32("volume", volume);
		BString title;
		if (volume == 0)
			title = B_TRANSLATE("Mute");
		else
			title << (volume * 10) << " %";
		fSoundVolumeMenu->AddItem(new BMenuItem(title, message));
	}
	fSoundVolumeMenu->SetRadioMode(true);
	menu->AddItem(fSoundVolumeMenu);
	menu->AddSeparatorItem();
	fJoystickMenu = new BMenu(B_TRANSLATE("Joystick"));
	fJoystickMenu->AddItem(new BMenuItem(B_TRANSLATE("Cursor"), new BMessage(kDeviceJoystickCursor)));
	fJoystickMenu->AddItem(new BMenuItem(B_TRANSLATE("Cursor + Enter"), new BMessage(kDeviceJoystickCursorEnter)));
	fJoystickMenu->AddItem(new BMenuItem(B_TRANSLATE("Kempston"), new BMessage(kDeviceJoystickKempston)));
	fJoystickMenu->AddItem(new BMenuItem(B_TRANSLATE("Sinclair 2"), new BMessage(kDeviceJoystickSinclair2)));
	fJoystickMenu->AddItem(new BMenuItem(B_TRANSLATE("QAOPM"), new BMessage(kDeviceJoystickQAOPM)));
	fJoystickMenu->AddItem(new BMenuItem(B_TRANSLATE("QAOP + Space"), new BMessage(kDeviceJoystickQAOPSpace)));
	fJoystickMenu->SetRadioMode(true);
	menu->AddItem(fJoystickMenu);
	menu->AddSeparatorItem();
	fKempstonMouseGrabMenuItem = new BMenuItem(B_TRANSLATE("Kempson mouse grab"), new BMessage(kDeviceKempsonMouse), 'G');
	menu->AddItem(fKempstonMouseGrabMenuItem);
	menu->AddSeparatorItem();
	f48kModeMenuItem = new BMenuItem(B_TRANSLATE("Mode 48k"), new BMessage(kDeviceMode48k), 'M');
	menu->AddItem(f48kModeMenuItem);
	menu->AddSeparatorItem();
	fPauseMenuItem = new BMenuItem(B_TRANSLATE("Pause"), new BMessage(kDevicePause), 'P');
	menu->AddItem(fPauseMenuItem);
	fResetToServiceROMMenuItem = new BMenuItem(B_TRANSLATE("Reset to service ROM"), new BMessage(kDeviceResetToServiceROM));
	menu->AddItem(fResetToServiceROMMenuItem);
	menu->AddItem(new BMenuItem(B_TRANSLATE("Reset"), new BMessage(kDeviceReset), 'R'));
	fMenuBar->AddItem(menu);
	menu = new BMenu(B_TRANSLATE("Help"));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Register filetypes"), new BMessage(kHelpRegisterMime)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("Visit homepage"), new BMessage(kHelpOpenHomePage)));
	menu->AddSeparatorItem();
	menu->AddItem(new BMenuItem(B_TRANSLATE("About"), new BMessage(B_ABOUT_REQUESTED)));
	fMenuBar->AddItem(menu);

	AddShortcut('F', B_COMMAND_KEY, new BMessage(kViewFullScreen));

	fView = new SurfaceView(Bounds(), w, h);
	fView->SetExplicitMinSize(BSize(xUi::WIDTH, xUi::HEIGHT));
	fView->SetViewColor(B_TRANSPARENT_COLOR);

	BFont font(be_plain_font);
	font.SetSize(ceilf(font.Size() * 0.75));

	fStatusMode = new BStringView("status", B_TRANSLATE("128k"));
	fStatusMode->SetFont(&font, B_FONT_SIZE);
	fStatusMode->SetAlignment(B_ALIGN_LEFT);

	fStatusDisk = new BStringView("disk status", "[A] b c d");
	fStatusDisk->SetFont(&font, B_FONT_SIZE);
	fStatusDisk->SetAlignment(B_ALIGN_LEFT);

	fStatusJoystick = new BStringView("joystick status", B_TRANSLATE("Cursor"));
	fStatusJoystick->SetFont(&font, B_FONT_SIZE);
	fStatusJoystick->SetAlignment(B_ALIGN_LEFT);

	fStatusSound = new BStringView("sound status", "AY:ABC");
	fStatusSound->SetFont(&font, B_FONT_SIZE);
	fStatusSound->SetAlignment(B_ALIGN_LEFT);

	fStatusInfo = new BStringView("status info", B_TRANSLATE("Ready"));
	fStatusInfo->SetFont(&font, B_FONT_SIZE);
	fStatusInfo->SetAlignment(B_ALIGN_LEFT);

	const float kElementSpacing = 2;

	BGroupLayout* statusGroup = BLayoutBuilder::Group<>(B_VERTICAL, 0.0)
		.Add(new BSeparatorView(B_HORIZONTAL, B_PLAIN_BORDER))
		.Add(BLayoutBuilder::Group<>(B_HORIZONTAL, kElementSpacing)
			.Add(fStatusMode)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fStatusDisk)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fStatusJoystick)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fStatusSound)
			.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
			.Add(fStatusInfo)
			.AddGlue()
			.SetInsets(kElementSpacing, 0, kElementSpacing, 0)
		);

	const float spacing = be_control_look->DefaultItemSpacing();
	BGroupLayout* pathLayout;
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(fMenuBar)
		.AddGroup(B_VERTICAL, B_USE_ITEM_SPACING)
			.Add(fView)		
		.End()
		.Add(statusGroup);

	VolumeAudio(OpVolume());
	UpdateMenus();
	UpdateCursor();

	fTimerMessageRunner = new BMessageRunner(BMessenger(this), new BMessage(kPulseEvent), 1000000);
}

HaikuPlatformWindow::~HaikuPlatformWindow()
{
	char buf[PATH_MAX];

	if (!fFullscreen) {
		int x = Frame().left;
		int y = Frame().top;
		int w = Frame().Width();
		int h = Frame().Height();

		sprintf(buf, op_window_state.FormatStr(), x, y, w, h);
		op_window_state.Value(buf);
	}

	if (fKeyboardWindow) {
		int x = fKeyboardWindow->Frame().left;
		int y = fKeyboardWindow->Frame().top;

		sprintf(buf, op_keyboard_window_state.FormatStr(), x, y);
		op_keyboard_window_state.Value(buf);

		fKeyboardWindow->Lock();
		fKeyboardWindow->Quit();
	}
	
	delete fTimerMessageRunner;
}

void
HaikuPlatformWindow::SetStatusInfo(const char *text, int time, color_which color)
{
	fStateInfoTimer = time;
	fStatusInfo->SetHighColor(ui_color(color));
	fStatusInfo->SetText(text);
}

void
HaikuPlatformWindow::UpdateMenus()
{
	xOptions::eOption<bool>* op_reset_to_service_rom = xOptions::eOption<bool>::Find("reset to service rom");
	xOptions::eOption<bool>* op_mode_48k = xOptions::eOption<bool>::Find("mode 48k");
	xOptions::eOption<bool>* op_auto_play = xOptions::eOption<bool>::Find("auto play image");
	xOptions::eOption<bool>* op_tape_fast = xOptions::eOption<bool>::Find("fast tape");
	xOptions::eOption<int>* op_sound_chip = xOptions::eOption<int>::Find("sound chip");
	xOptions::eOption<int>* op_ay_stereo = xOptions::eOption<int>::Find("ay stereo");
	xOptions::eOption<int>* op_save_slot = xOptions::eOption<int>::Find("save slot");

	eDrive drive = OpDrive();
	fBetaDiskMenu->FindItem(kDeviceBetaDiskA)->SetMarked(drive == D_A);
	fBetaDiskMenu->FindItem(kDeviceBetaDiskB)->SetMarked(drive == D_B);
	fBetaDiskMenu->FindItem(kDeviceBetaDiskC)->SetMarked(drive == D_C);
	fBetaDiskMenu->FindItem(kDeviceBetaDiskD)->SetMarked(drive == D_D);

	f48kModeMenuItem->SetMarked(op_mode_48k && *op_mode_48k);
	fResetToServiceROMMenuItem->SetMarked(op_reset_to_service_rom && *op_reset_to_service_rom);
	fFilteringMenuItem->SetMarked(op_scale_bilinear);
	fXBRFilteringMenuItem->SetMarked(op_xbr_scale);
	fSmartBorderScaleMenuItem->SetMarked(!op_smart_border_disabled);
	fVirtualKeyboardMenuItem->SetMarked(op_virtual_keyboard);
	fAutoPlayMenuItem->SetMarked(op_auto_play && *op_auto_play);
	fFastTapeMenuItem->SetMarked(op_tape_fast && *op_tape_fast);

	if (op_save_slot) {
		int index = *op_save_slot - 1;		
		if (BMenuItem *item = fQuickSaveSlotMenu->ItemAt(index))
			item->SetMarked(true);
	}

	if (op_sound_chip) {
		fSoundChipMenu->FindItem(kDeviceSoundChipAY)->SetMarked(*op_sound_chip == 0);
		fSoundChipMenu->FindItem(kDeviceSoundChipYM)->SetMarked(*op_sound_chip == 1);
	}

	if (op_ay_stereo) {
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoABC)->SetMarked(*op_ay_stereo == 0);
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoACB)->SetMarked(*op_ay_stereo == 1);
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoBAC)->SetMarked(*op_ay_stereo == 2);
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoBCA)->SetMarked(*op_ay_stereo == 3);
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoCAB)->SetMarked(*op_ay_stereo == 4);
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoCBA)->SetMarked(*op_ay_stereo == 5);
		fAYStereoMenu->FindItem(kDeviceSoundChipStereoMono)->SetMarked(*op_ay_stereo == 6);
	}

	if (BMenuItem *item = fSoundVolumeMenu->ItemAt(OpVolume()))
		item->SetMarked(true);

	eJoystick joy = OpJoystick();
	fJoystickMenu->FindItem(kDeviceJoystickCursor)->SetMarked(joy == J_CURSOR);
	fJoystickMenu->FindItem(kDeviceJoystickCursorEnter)->SetMarked(joy == J_CURSORENTER);
	fJoystickMenu->FindItem(kDeviceJoystickKempston)->SetMarked(joy == J_KEMPSTON);
	fJoystickMenu->FindItem(kDeviceJoystickSinclair2)->SetMarked(joy == J_SINCLAIR2);
	fJoystickMenu->FindItem(kDeviceJoystickQAOPM)->SetMarked(joy == J_QAOPM);
	fJoystickMenu->FindItem(kDeviceJoystickQAOPSpace)->SetMarked(joy == J_QAOPSPACE);

	fKempstonMouseGrabMenuItem->SetMarked(fKempsonMouseGrab);

	// update status line
	if (op_mode_48k && *op_mode_48k)
		fStatusMode->SetText(B_TRANSLATE("48k"));
	else
		fStatusMode->SetText(B_TRANSLATE("128k"));

	if (drive == D_A)fStatusDisk->SetText("[A] b c d");
	if (drive == D_B)fStatusDisk->SetText("a [B] c d");
	if (drive == D_C)fStatusDisk->SetText("a b [C] d");
	if (drive == D_D)fStatusDisk->SetText("a b c [D]");

	if (BMenuItem *item = fJoystickMenu->FindMarked())
		fStatusJoystick->SetText(item->Label());

	BString soundStatus;
	soundStatus += (op_sound_chip && *op_sound_chip == 0) ? "AY: " : "YM: ";

	if (BMenuItem *item = fAYStereoMenu->FindMarked())
		soundStatus << item->Label();
	else
		soundStatus << "ABC";

	BString volumeTxt;
	volumeTxt << (OpVolume() * 10) << "%";
	soundStatus += " ";
	soundStatus += ((OpVolume() == 0) ? B_TRANSLATE("Mute") : volumeTxt.String());
	fStatusSound->SetText(soundStatus);
}

void
HaikuPlatformWindow::UpdateCursor()
{
	BCursor cursor((fFullscreen || fKempsonMouseGrab) ? B_CURSOR_ID_NO_CURSOR : B_CURSOR_ID_SYSTEM_DEFAULT);
	fView->SetViewCursor(&cursor);
}

void
HaikuPlatformWindow::ResizeToScale(int scale)
{
	if (scale == 0) {
		fSkipScale = true;
		if (fFullscreen) {
			MoveTo(fLastRect.left, fLastRect.top);
			ResizeTo(fLastRect.Width(), fLastRect.Height());
		} else {
			fLastRect = Frame();
			BScreen scr(B_MAIN_SCREEN_ID);
			float height = fMenuBar->Bounds().Height() + fStatusMode->Bounds().Height() + 2;
			MoveTo(0, -(fMenuBar->Bounds().Height() + 1));
			ResizeTo(scr.Frame().Width() + 1, scr.Frame().Height() + height + 2);
		}
		fScaleFactor = -1;
		fFullscreen = !fFullscreen;
		op_full_screen.Set(fFullscreen);
		UpdateCursor();
	}
	if (scale >= 1 && scale <= 3) {
		if (scale != fScaleFactor) {
			fSkipScale = true;

			if (fFullscreen)
				MoveTo(fLastRect.left, fLastRect.top);

			int w = xUi::WIDTH * scale;
			int h = xUi::HEIGHT * scale;
			h += fMenuBar->Bounds().Height() + fStatusMode->Bounds().Height() + 2;
			ResizeTo(w + 1, h + 1);
			fScaleFactor = scale;
			fFullscreen = false;
		}
	}
}

void
HaikuPlatformWindow::InstallMimeType(const char *_mime, const char *_icon, const char *_description, const char *_exts)
{
	// install mime type of document
	BMimeType mime(_mime);
	status_t ret = mime.InitCheck();
	if (ret != B_OK) {
		fprintf(stderr, B_TRANSLATE("Could not init native document mime type (%s): %s.\n"),
			_mime, strerror(ret));
		return;
	}

	ret = mime.Install();
	if (ret != B_OK && ret != B_FILE_EXISTS) {
		fprintf(stderr, B_TRANSLATE("Could not install native document mime type (%s): %s.\n"),
			_mime, strerror(ret));
		return;
	}

	mime.SetPreferredApp(APP_SIGNATURE);
	mime.SetShortDescription(_description);

	// set extensions
	BString extensions(_exts);
	BStringList extList;
	extensions.Split(",", true, extList);
	BMessage message;
	for (int i = 0; i < extList.CountStrings(); i++)
		message.AddString("extensions", extList.StringAt(i));
	mime.SetFileExtensions(&message);

	// set mime icon
	BResources* resources = be_app->AppResources();
	if (resources != NULL) {
		size_t size;
		const void* iconData = resources->LoadResource('VICN', _icon, &size);
		if (iconData != NULL && size > 0) {
			if (mime.SetIcon(reinterpret_cast<const uint8*>(iconData), size) != B_OK)
				fprintf(stderr, B_TRANSLATE("Could not set vector icon of mime type.\n"));
		} else {
			fprintf(stderr, B_TRANSLATE("Could not find icon in app resources (data: %p, size: %ld).\n"), iconData, size);
		}
	} else
		fprintf(stderr, B_TRANSLATE("Could not find app resources.\n"));
}

void 
HaikuPlatformWindow::MessageReceived(BMessage *message)
{
	if (message->WasDropped())
		message->what = B_REFS_RECEIVED;

	switch (message->what) {
		case kPulseEvent:
		{
			if (fStateInfoTimer > 0) {
				fStateInfoTimer--;
				if (fStateInfoTimer == 0)
					SetStatusInfo(B_TRANSLATE("Ready"));
			}
			break;
		}
		case kFileOpen:
		{
			if (!fOpenPanel) {
				fOpenPanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL, 0, true, NULL, NULL, true, true);
				fOpenPanel->Window()->SetTitle(B_TRANSLATE("Open file" B_UTF8_ELLIPSIS));
				fOpenPanel->SetTarget(this);
			}
			Handler()->VideoPaused(true);
			fOpenPanel->Show();
			break;
		}
		case kFileOpenWeb:
		{
			if (!fWebWindow) {
				fWebWindow = new WebWindow(BRect(100, 100, 640, 480), this);
			}
			if (fWebWindow->IsHidden())
				fWebWindow->Show();
			break;
		}
		case kFileQuickOpen:
		{
			using namespace xOptions;
			eOption<bool>* loadState = eOption<bool>::Find("load state");
			if(loadState)
				loadState->Change();
			if (*loadState) {
				SetStatusInfo(B_TRANSLATE("Quick load OK"), STATUS_TIME_DEFAULT);
				fQuickSaveMenuItem->SetEnabled(true);
			} else
				SetStatusInfo(B_TRANSLATE("Quick load FAILED"), STATUS_TIME_DEFAULT, B_FAILURE_COLOR);
			break;
		}
		case kFileQuickSave:
		{
			using namespace xOptions;
			eOption<bool>* saveState = eOption<bool>::Find("save state");
			if(saveState) {
				Handler()->VideoPaused(true);
				saveState->Change();
				Handler()->VideoPaused(false);
			}
			if (*saveState)
				SetStatusInfo(B_TRANSLATE("Quick save OK"), STATUS_TIME_DEFAULT);
			else
				SetStatusInfo(B_TRANSLATE("Quick save FAILED"), STATUS_TIME_DEFAULT, B_FAILURE_COLOR);
			break;
		}
		case kFileQuickSaveSlot:
		{
			using namespace xOptions;
			eOption<int>* saveStateSlot = eOption<int>::Find("save slot");
			int32 slot = message->FindInt32("slot");
			if(saveStateSlot)
				saveStateSlot->Set(slot);
			UpdateMenus();
			break;
		}
		case kFileSave:
		{
			if (!fSavePanel) {
				fSavePanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL, 0, true, NULL, NULL, true, true);
				fSavePanel->Window()->SetTitle(B_TRANSLATE("Save SNA snapshot" B_UTF8_ELLIPSIS));
				fSavePanel->SetTarget(this);
			}
			Handler()->VideoPaused(true);
			fSavePanel->Show();
			break;
		}
		case kFileSaveScreen:
		{
			if (!fSaveScreenPanel) {
				fSaveScreenPanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL, 0, true, NULL, NULL, true, true);
				fSaveScreenPanel->SetMessage(new BMessage(B_SAVE_SCREEN_REQUESTED));
				fSaveScreenPanel->Window()->SetTitle(B_TRANSLATE("Save screenshot" B_UTF8_ELLIPSIS));
				fSaveScreenPanel->SetTarget(this);
			}
			Handler()->VideoPaused(true);
			fSaveScreenPanel->Show();
			break;
		}
		case kFileAutoPlay:
		{
			if(UpdateBoolOption(fAutoPlayMenuItem, "auto play image", true)) {
				SetStatusInfo(B_TRANSLATE("Auto launch on"), STATUS_TIME_DEFAULT);
			} else {
				SetStatusInfo(B_TRANSLATE("Auto launch off"), STATUS_TIME_DEFAULT);
			}
			UpdateMenus();
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (message->FindRef("refs", 0, &ref) == B_OK) {
				BEntry entry(&ref, true);
				if (!entry.Exists())
					break;
				BPath path;
				if (entry.GetPath(&path) != B_OK)
					break;

				BString text;
				bool status = Handler()->OnOpenFile(path.Path());
				if (status) {
					text << B_TRANSLATE("File was successfully opened");
					fQuickSaveMenuItem->SetEnabled(true);
				} else
					text << B_TRANSLATE("Could not open file");

				SetStatusInfo(text.String(), STATUS_TIME_DEFAULT, status ? B_PANEL_TEXT_COLOR : B_FAILURE_COLOR);
			}
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref dir;
			BDirectory directory;
			if (message->FindRef("directory", &dir) == B_OK) {
				BString name = message->FindString("name");
				directory.SetTo(&dir);
				if (directory.InitCheck() == B_NO_ERROR) {
					BPath pathname(&dir);
					if (!name.IEndsWith(".sna"))
						name += ".sna";
					pathname.Append(name);
					if(Handler()->OnSaveFile(pathname.Path()))
						SetStatusInfo(B_TRANSLATE("File save OK"), STATUS_TIME_DEFAULT);
					else
						SetStatusInfo(B_TRANSLATE("File save FAILED"), STATUS_TIME_DEFAULT, B_FAILURE_COLOR);
				}
			}
			break;
		}
		case B_SAVE_SCREEN_REQUESTED:
		{
			entry_ref dir;
			BDirectory directory;
			if (message->FindRef("directory", &dir) == B_OK) {
				BString name = message->FindString("name");
				directory.SetTo(&dir);
				if (directory.InitCheck() == B_NO_ERROR) {
					BPath pathname(&dir);
					if (!name.IEndsWith(".png"))
						name += ".png";
					pathname.Append(name);
					if (Handler()->OnSaveFile(pathname.Path()))
						SetStatusInfo(B_TRANSLATE("Screenshot save OK"), STATUS_TIME_DEFAULT);
					else
						SetStatusInfo(B_TRANSLATE("Screenshot save FAILED"), STATUS_TIME_DEFAULT, B_FAILURE_COLOR);
				}
			}
			break;
		}
		case B_OK:
		case B_CANCEL:
		{
			Handler()->VideoPaused(false);
			break;
		}
		case kViewScale1x:
		{
			ResizeToScale(1);
			break;
		}
		case kViewScale2x:
		{
			ResizeToScale(2);
			break;
		}
		case kViewScale3x:
		{
			ResizeToScale(3);
			break;
		}
		case kViewFullScreen:
		{
			ResizeToScale(0);
			break;
		}		
		case kViewFiltering:
		{
			fFiltering = !fFiltering;
			fView->EnableBilinear(fFiltering);
			op_scale_bilinear.Set(fFiltering);
			SetStatusInfo(fFiltering ? B_TRANSLATE("Filtering enabled") : B_TRANSLATE("Filtering disabled"), STATUS_TIME_DEFAULT);
			UpdateMenus();
			break;
		}
		case kViewXBRFiltering:
		{
			fXBRFiltering = !fXBRFiltering;
			fView->EnableXBR(fXBRFiltering);
			op_xbr_scale.Set(fXBRFiltering);
			SetStatusInfo(fXBRFiltering ? B_TRANSLATE("XBR Filtering enabled") : B_TRANSLATE("XBR Filtering disabled"), STATUS_TIME_DEFAULT);
			UpdateMenus();
			break;
		}
		case kViewSmartBorder:
		{
			op_smart_border_disabled.Set(!op_smart_border_disabled);
			fView->EnableSmartBorder(!op_smart_border_disabled);
			SetStatusInfo(op_smart_border_disabled ? B_TRANSLATE("Smart border disabled") : B_TRANSLATE("Smart border enabled"), STATUS_TIME_DEFAULT);
			UpdateMenus();
			break;
		}
		case kViewOnScreenKeyboard:
		{
			Handler()->OnKey('k', KF_DOWN);
			Handler()->OnKey('k', 0);
			break;
		}
		case kViewOnScreenMenu:
		{
			Handler()->OnKey('m', KF_DOWN);
			Handler()->OnKey('m', 0);
			break;
		}
		case kDeviceStartStopTape:
		{
			switch(Handler()->OnAction(A_TAPE_TOGGLE))
			{
				case AR_TAPE_STARTED:
					SetStatusInfo(B_TRANSLATE("Tape started"), STATUS_TIME_DEFAULT);
					break;
				case AR_TAPE_STOPPED:
					SetStatusInfo(B_TRANSLATE("Tape stopped"), STATUS_TIME_DEFAULT);
					break;
				case AR_TAPE_NOT_INSERTED:
					SetStatusInfo(B_TRANSLATE("Tape not inserted"), STATUS_TIME_DEFAULT);
					break;
				default:
					SetStatusInfo(B_TRANSLATE("Ready"));
					break;
			}
			break;
		}
		case kDeviceFastTape:
		{
			using namespace xOptions;
			eOption<bool>* op_tape_fast = eOption<bool>::Find("fast tape");
			SAFE_CALL(op_tape_fast)->Change();
			bool tape_fast = op_tape_fast && *op_tape_fast;
			SetStatusInfo(tape_fast ? B_TRANSLATE("Fast tape on") : B_TRANSLATE("Fast tape off"), STATUS_TIME_DEFAULT);
			UpdateMenus();
			break;
		}
		case kDeviceBetaDiskA:
		{
			OpDrive(D_A);
			UpdateMenus();
			break;
		}
		case kDeviceBetaDiskB:
		{
			OpDrive(D_B);
			UpdateMenus();
			break;
		}
		case kDeviceBetaDiskC:
		{
			OpDrive(D_C);
			UpdateMenus();
			break;
		}
		case kDeviceBetaDiskD:
		{
			OpDrive(D_D);
			UpdateMenus();
			break;
		}
		case kDeviceSoundVolume:
		{
			using namespace xOptions;
			eOption<int>* op_volume = xOptions::eOption<int>::Find("volume");
			uint32 volume = message->FindInt32("volume");
			op_volume->Set(volume);
			VolumeAudio(volume);
			UpdateMenus();
			break;
		}
		case kDeviceSoundChipAY:
		{
			using namespace xOptions;
			eOption<int>* op_sound_chip = eOption<int>::Find("sound chip");
			op_sound_chip->Set(0);
			UpdateMenus();
			break;
		}
		case kDeviceSoundChipYM:
		{
			using namespace xOptions;
			eOption<int>* op_sound_chip = eOption<int>::Find("sound chip");
			op_sound_chip->Set(1);
			UpdateMenus();
			break;
		}
		case kDeviceSoundChipStereoABC:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(0);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}		
		case kDeviceSoundChipStereoACB:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(1);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}		
		case kDeviceSoundChipStereoBAC:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(2);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}		
		case kDeviceSoundChipStereoBCA:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(3);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}		
		case kDeviceSoundChipStereoCAB:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(4);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}		
		case kDeviceSoundChipStereoCBA:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(5);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}		
		case kDeviceSoundChipStereoMono:
		{
			using namespace xOptions;
			eOption<int>* op_ay_stereo = eOption<int>::Find("ay stereo");
			op_ay_stereo->Set(6);
			op_ay_stereo->Apply();
			UpdateMenus();
			break;
		}
		case kDeviceJoystickCursor:
		{
			OpJoystick(J_CURSOR);
			UpdateMenus();
			break;
		}
		case kDeviceJoystickCursorEnter:
		{
			OpJoystick(J_CURSORENTER);
			UpdateMenus();
			break;
		}
		case kDeviceJoystickKempston:
		{
			OpJoystick(J_KEMPSTON);
			UpdateMenus();
			break;
		}
		case kDeviceJoystickSinclair2:
		{
			OpJoystick(J_SINCLAIR2);
			UpdateMenus();
			break;
		}
		case kDeviceJoystickQAOPM:
		{
			OpJoystick(J_QAOPM);
			UpdateMenus();
			break;
		}
		case kDeviceJoystickQAOPSpace:
		{
			OpJoystick(J_QAOPSPACE);
			UpdateMenus();
			break;
		}
		case kDeviceKempsonMouse:
		{
			fKempsonMouseGrab = !fKempsonMouseGrab;
			fView->EnableMouseGrab(fKempsonMouseGrab);
			SetStatusInfo(fKempsonMouseGrab ? B_TRANSLATE("Mouse grabbed (Alt+G)") : B_TRANSLATE("Ready"));
			UpdateMenus();
			UpdateCursor();
			break;
		}
		case kDevicePause:
		{
			bool marked = fPauseMenuItem->IsMarked();
			fPauseMenuItem->SetMarked(!marked);
			Handler()->VideoPaused(!marked);
			SetStatusInfo(marked ? B_TRANSLATE("Ready") : B_TRANSLATE("Paused"));
			break;
		}
		case kDeviceReset:
		{
			SetStatusInfo(B_TRANSLATE("Reset"), STATUS_TIME_DEFAULT);
			Handler()->OnAction(A_RESET);
			break;
		}
		case kDeviceMode48k:
		{
			if (UpdateBoolOption(f48kModeMenuItem, "mode 48k", true))
				SetStatusInfo(B_TRANSLATE("48k mode enabled"), STATUS_TIME_DEFAULT);
			else
				SetStatusInfo(B_TRANSLATE("128k mode enabled"), STATUS_TIME_DEFAULT);
			UpdateMenus();
			break;
		}
		case kDeviceResetToServiceROM:
		{
			if (UpdateBoolOption(fResetToServiceROMMenuItem, "reset to service rom", true))
				SetStatusInfo(B_TRANSLATE("Reset to service ROM"), STATUS_TIME_DEFAULT);
			else
				SetStatusInfo(B_TRANSLATE("Reset to usual ROM"), STATUS_TIME_DEFAULT);
			break;
		}
		case kHelpKeyboardLayout:
		{
			if (!fKeyboardWindow) {
				int x, y;
				BRect rect(100, 100, 950, 450);
				if (sscanf(op_keyboard_window_state, op_keyboard_window_state.FormatStr(), &x, &y) == 2)
					rect.OffsetTo(x, y);	
				fKeyboardWindow = new KeyboardWindow(rect, this);
			}
			if (fKeyboardWindow->IsHidden()) {
				fKeyboardWindow->Show();
				Activate(true);
				op_virtual_keyboard.Set(true);
			} else {
				fKeyboardWindow->Hide();
				op_virtual_keyboard.Set(false);
			}
			UpdateMenus();
			break;
		}
		case kHelpRegisterMime:
		{
			InstallMimeType("application/x-spectrum-sna", "snapshot_icon", "ZX-Spectrum emulator snapshot (SNA)", "sna");
			InstallMimeType("application/x-spectrum-z80", "snapshot_icon", "ZX-Spectrum emulator snapshot (Z80)", "z80");
			InstallMimeType("application/x-spectrum-trd", "disk_icon", "TR-DOS floppy disk image (TRD)", "trd");
			InstallMimeType("application/x-spectrum-udi", "disk_icon", "TR-DOS floppy disk image (UDI)", "udi");
			InstallMimeType("application/x-spectrum-fdi", "disk_icon", "TR-DOS floppy disk image (FDI)", "fdi");
			InstallMimeType("application/x-spectrum-scl", "disk_icon", "TR-DOS floppy disk image (SCL)", "scl");
			InstallMimeType("application/x-spectrum-tap", "tape_icon", "ZX-Spectrum emulator tape data (TAP)", "tap");
			InstallMimeType("application/x-spectrum-tzx", "tape_icon", "ZX-Spectrum emulator tape data (TZX)", "tzx");
			BAlert *alert = new BAlert(B_TRANSLATE("Register filetypes"), B_TRANSLATE("Filetypes registered!"), B_TRANSLATE("OK"));
			alert->Go(NULL);
			break;
		}
		case kHelpOpenHomePage:
		{
			entry_ref ref;
			get_ref_for_path("/bin/open", &ref);
			const char* args[] = { "/bin/open", "https://bitbucket.org/djdron/unrealspeccyp", NULL };
			be_roster->Launch(&ref, 2, args);
			break;
		}
		case B_ABOUT_REQUESTED:
		{
			BAboutWindow* wind = new BAboutWindow("Unreal Speccy Portable", APP_SIGNATURE);
			const char *backendCopyrights[]={"2021-2023 Haiku UI by Gerasim Troeglazov (3dEyes**)\n", NULL};
			wind->AddCopyright(2022, "SMT, Dexus, Alone Coder, deathsoft, djdron, scor.", (const char**)&backendCopyrights);
			wind->SetVersion("0.0.86.18");
			wind->AddDescription("Portable ZX Spectrum emulator.");
			wind->AddExtraInfo(
"This program is free software: you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation, either version 3 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");
			wind->ResizeTo(wind->Frame().Width() * 1.4, wind->Frame().Height() * 1.8);
			wind->CenterOnScreen();
			wind->Show();
			break;
		}
		case B_UNMAPPED_KEY_UP:
		case B_KEY_UP:
		{
			int32 key, mod;
			message->FindInt32("key", &key);
			message->FindInt32("modifiers", &mod);

			if (fKeyboardWindow) {
				message->AddBool("highlight", false);
				message->what = kKeyHighlight;
				fKeyboardWindow->PostMessage(message);
			}

			dword flags = 0;
			if(mod & B_CONTROL_KEY)
				flags |= KF_ALT;
			if(mod & B_SHIFT_KEY)
				flags |= KF_SHIFT;
			if (key == 0x26) {
				Handler()->OnKey('c', 0);
				Handler()->OnKey('s', 0);
			} else if (key == 0x31) {
				SEND_EXTEND_KEY('F', 'Y', flags);
			} else if (key == 0x32) {
				SEND_EXTEND_KEY('G', 'U', flags);
			} else if (key == 0x33) {
				SEND_EXTEND_KEY('S', 'D', flags);
			} else {
				if (!CheckFuncKey(key, flags)) {
					byte keycode = TranslateKey(key, fLastPressedKey, flags);
					Handler()->OnKey(keycode, flags);
					if ((keycode >= '0' && keycode <= '9') || (keycode >= 'A' && keycode <= 'Z'))
						fLastPressedKey = 0;
					if (keycode == 'm' || (keycode == 'e' && !Handler()->VideoDataUI()))
						UpdateMenus();
				}
			}
			break;
		}
		case B_UNMAPPED_KEY_DOWN:
		case B_KEY_DOWN:
		{
			int32 key, mod;
			message->FindInt32("key", &key);
			message->FindInt32("modifiers", &mod);
			printf("key: %x\n", key);

			if (fKeyboardWindow) {
				message->AddBool("highlight", true);
				message->what = kKeyHighlight;
				fKeyboardWindow->PostMessage(message);
			}

			dword flags = KF_DOWN;
			if(mod & B_CONTROL_KEY)
				flags |= KF_ALT;
			if(mod & B_SHIFT_KEY)
				flags |= KF_SHIFT;			
			if (key == 0x26 || key == 0x31 || key == 0x32 || key == 0x33) {
				Handler()->OnKey('c', KF_DOWN | KF_ALT | KF_SHIFT);
				Handler()->OnKey('s', KF_DOWN | KF_ALT | KF_SHIFT);
			} else {
				if (!CheckFuncKey(key, flags)) {
					byte keycode = TranslateKey(key, fLastPressedKey, flags);
					Handler()->OnKey(keycode, flags);
					if ((keycode >= '0' && keycode <= '9') || (keycode >= 'A' && keycode <= 'Z'))
						fLastPressedKey = keycode;
				}
			}
			break;
		}
		default:
			break;
	}
	BWindow::MessageReceived(message);
}

byte
HaikuPlatformWindow::TranslateKey(uint32 _key, byte _last, dword& _flags)
{
	byte* data_ui = (byte*)Handler()->VideoDataUI();

	switch(_key)
	{
	case 0x4B:	// Left Shift
	case 0x56:	// Right Shift
		return 'c';
	case 0x5c:	// Left Control
	case 0x60:	// Right Control
		return 's';
	case 0x47:	// Enter
		return 'e';
	case 0x34:	// Delete
	case 0x1E:	// Backspace
		if (_flags & KF_DOWN)
			_flags |= KF_SHIFT;
		return '0';
	case 0x1F:	// Insert
		if (_flags & KF_DOWN)
			_flags |= KF_SHIFT;
		return '1';
	case 0x20:	// Home
		if (_flags & KF_DOWN)
			_flags |= KF_SHIFT;
		return '3';
	case 0x35:	// End
		if (_flags & KF_DOWN)
			_flags |= KF_SHIFT;
		return '4';
	case 0x66:	// Left Win
	case 0x67:	// Right Win
		if (_flags & KF_DOWN)
			_flags |= KF_SHIFT;
		return '9';
	case 0x46:	//	'"
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == 'P') {
			_flags &= ~KF_SHIFT;
			return 'P';
		} else
			return '7';
	case 0x53:	// ,<
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == 'R')
		{
			_flags &= ~KF_SHIFT;
			return 'R';
		}
		else
			return 'N';
	case 0x54:	// .>
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == 'T')
		{
			_flags &= ~KF_SHIFT;
			return 'T';
		}
		else
			return 'M';
	case 0x55:	// ?/
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == 'C')
		{
			_flags &= ~KF_SHIFT;
			return 'C';
		}
		else
			return 'V';
	case 0x45:	// :;
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == 'Z')
		{
			_flags &= ~KF_SHIFT;
			return 'Z';
		}
		else
			return 'O';
	case 0x1C:	// -_
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == '0')
		{
			_flags &= ~KF_SHIFT;
			return '0';
		}
		else
			return 'J';
	case 0x1D:	// +=
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		if(_flags & KF_SHIFT || _last == 'K')
		{
			_flags &= ~KF_SHIFT;
			return 'K';
		}
		else
			return 'L';
	case 0x3b:	// Caps Lock
		if (_flags & KF_DOWN)
			_flags |= KF_SHIFT;
		return '2';
	case 0x11:	// `
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		return '7';

	case 0x23:	// NumPad /
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		return 'V';
	case 0x24:	// NumPad *
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		return 'B';
	case 0x25:	// NumPad -
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		return 'J';
	case 0x3a:	// NumPad +
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		return 'K';
	case 0x65:	// NumPad .
		if (_flags & KF_DOWN)
			_flags |= KF_ALT;
		return 'M';

	case 0x27: return 'Q';
	case 0x28: return 'W';
	case 0x29: return 'E';
	case 0x2A: return 'R';
	case 0x2B: return 'T';
	case 0x2C: return 'Y';
	case 0x2D: return 'U';
	case 0x2E: return 'I';
	case 0x2F: return 'O';
	case 0x30: return 'P';

	case 0x3C: return 'A';
	case 0x3D: return 'S';
	case 0x3E: return 'D';
	case 0x3F: return 'F';
	case 0x40: return 'G';
	case 0x41: return 'H';
	case 0x42: return 'J';
	case 0x43: return 'K';
	case 0x44: return 'L';

	case 0x4C: return 'Z';
	case 0x4D: return 'X';
	case 0x4E: return 'C';
	case 0x4F: return 'V';
	case 0x50: return 'B';
	case 0x51: return 'N';
	case 0x52: return 'M';

	default:
		break;		
	}
	// Cursor
	if (data_ui) {
		switch(_key) {
			case 0x01:
				if (_flags & KF_DOWN)
					PostMessage(kViewOnScreenMenu);
				return 0;
			case 0x61:		return 'l';
			case 0x63:		return 'r';
			case 0x57:		return 'u';
			case 0x62:		return 'd';
			default:
				break;
		}
	} else {
		switch(_key) {
			case 0x01:	// ESC
				if (_flags & KF_DOWN)
					_flags |= KF_SHIFT;
				return ' ';
			case 0x61:  // Cursor Left
				if (_flags & KF_DOWN)
					_flags |= KF_SHIFT;
				return '5';
			case 0x63:  // Cursor Right
				if (_flags & KF_DOWN)
					_flags |= KF_SHIFT;
				return '8';
			case 0x57:  // Cursor Up
				if (_flags & KF_DOWN)
					_flags |= KF_SHIFT;
				return '7';
			case 0x62:  // Cursor Down
				if (_flags & KF_DOWN)
					_flags |= KF_SHIFT;
				return '6';
			default:
				break;
		}
	}
	// 0
	if(_key == 0x1B)
		return '0';
	// 1-9
	if(_key >= 0x12 && _key <= 0x1A)
		return (_key - 0x12) + '1';	
	// Space
	if(_key == 0x5E)
		return ' ';

	// NumPad
	if (modifiers() & B_NUM_LOCK) {
		switch (_key) {
			case 0x64:	// 0
				return '0';
			case 0x58:	// 1
				return '1';
			case 0x59:	// 2
				return '2';
			case 0x5a:	// 3
				return '3';
			case 0x48:	// 4
				return '4';
			case 0x49:	// 5
				return '5';
			case 0x4a:	// 6
				return '6';
			case 0x37:	// 7
				return '7';
			case 0x38:	// 8
				return '8';
			case 0x39:	// 9
				return '9';
			case 0x5b:	// Enter
				return 'e';
			default:
				break;
		}
	} else {
		switch (_key) {
			case 0x64:	// 0
			case 0x49:	// 5
			case 0x5b:	// Enter
				_flags |= OpJoyKeyFlags();
				return 'f';
			case 0x59:	// 2
				_flags |= OpJoyKeyFlags();
				return 'd';
			case 0x48:	// 4
				_flags |= OpJoyKeyFlags();
				return 'l';
			case 0x4a:	// 6
				_flags |= OpJoyKeyFlags();
				return 'r';
			case 0x38:	// 8
				_flags |= OpJoyKeyFlags();
				return 'u';
			default:
				break;
		}
	}

	return 0;
}

bool
HaikuPlatformWindow::CheckFuncKey(uint32 key, dword flags)
{
	if (!(flags & KF_DOWN))
		return false;

	switch(key) {
		case B_F1_KEY:	// F1
			PostMessage(B_ABOUT_REQUESTED);
			break;
		case B_F2_KEY:	// F2
			PostMessage(kFileSave);
			break;
		case B_F3_KEY:	// F3
			PostMessage(kFileOpen);
			break;
		case B_F4_KEY:	// F4
			PostMessage(kFileOpenWeb);
			break;
		case B_F5_KEY:	// F5
			PostMessage(kDeviceStartStopTape);
			break;
		case B_F6_KEY:	// F6
			PostMessage(kFileQuickSave);
			break;
		case B_F7_KEY:	// F7
			PostMessage(kFileQuickOpen);
			break;
		case B_F8_KEY:	// F7
			PostMessage(kHelpKeyboardLayout);
			break;
		case B_F9_KEY:
			PostMessage(kViewOnScreenKeyboard);
			break;
		case 0x68:		// Menu
		case B_F10_KEY: // F10
			PostMessage(kViewOnScreenMenu);
			break;
		case B_F11_KEY:	// F11
			PostMessage(kDeviceMode48k);
			break;
		case B_F12_KEY:	// F12
			PostMessage(kDeviceReset);
			break;
		case 0x10:		// Pause
			PostMessage(kDevicePause);
			break;
		case 0x21:		// PgUp
		{
			BMessage *message = new BMessage(kDeviceSoundVolume);
			int volume = OpVolume() > 9 ? 10 : OpVolume() + 1;
			message->AddInt32("volume", volume);
			PostMessage(message);
			break;
		}
		case 0x36:		// PgDown
		{
			BMessage *message = new BMessage(kDeviceSoundVolume);
			int volume = OpVolume() < 1 ? 0 : OpVolume() - 1;
			message->AddInt32("volume", volume);
			PostMessage(message);
			break;
		}
		default:
			return false;
	}
	return true;
}

void
HaikuPlatformWindow::FrameResized(float width, float height)
{
	BWindow::FrameResized(width, height);
	if (fSkipScale)
		fSkipScale = false;
	else
		fScaleFactor = -1;

	int multiplyIndex = ceilf(fView->Bounds().Width() / xUi::WIDTH) - 1;
	fXBRFilteringMenuItem->SetEnabled(multiplyIndex > 1);
}

void
HaikuPlatformWindow::WindowActivated(bool active)
{
}

void
HaikuPlatformWindow::Show()
{
	int w = xUi::WIDTH;
	int h = xUi::HEIGHT;
	fMenuBar->ResizeToPreferred();
	fStatusMode->ResizeToPreferred();	
	h += fMenuBar->Bounds().Height() + fStatusMode->Bounds().Height() + 2;
	SetSizeLimits(w, 4096, h, 4096);

	int x, y;
	if (sscanf(op_window_state, op_window_state.FormatStr(), &x, &y, &w, &h) == 4) {
		MoveTo(x, y);
		ResizeTo(w, h);
	} else
		PostMessage(kViewScale2x);

	if (op_scale_bilinear)
		PostMessage(kViewFiltering);
	if (op_xbr_scale)
		PostMessage(kViewXBRFiltering);
	if (op_full_screen)
		ResizeToScale(0);
	if (op_virtual_keyboard)
		PostMessage(kHelpKeyboardLayout);
	
	BMessage *refsMessage = ((USPApplication*)be_app)->GetRefsMessage();
	if (refsMessage)
		PostMessage(refsMessage);

	fView->EnableSmartBorder(!op_smart_border_disabled);

	BWindow::Show();
}

bool
HaikuPlatformWindow::QuitRequested()
{
	OpQuit(true);
	return false;
}

}
