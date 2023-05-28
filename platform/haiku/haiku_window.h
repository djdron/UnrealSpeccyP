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

#ifndef _HAIKU_PLATFORM_WINDOW_H
#define _HAIKU_PLATFORM_WINDOW_H

#include <stdio.h>

#include <AppKit.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <StorageKit.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>
#include <StringList.h>
#include <RecentItems.h>
#include <private/interface/AboutWindow.h>

#include "haiku_surface.h"
#include "haiku_keyboard_window.h"
#include "haiku_web_window.h"

const uint32 kFileOpen = 'FOPN';
const uint32 kFileSave = 'FSVE';
const uint32 kFileOpenWeb = 'FWEB';
const uint32 kFileQuickOpen = 'QOPN';
const uint32 kFileQuickSave = 'QSAV';
const uint32 kFileQuickSaveSlot = 'QSSL';
const uint32 kFileSaveScreen = 'QSVS';
const uint32 kFileAutoPlay = 'APLA';
const uint32 kViewScale1x = 'S100';
const uint32 kViewScale2x = 'S200';
const uint32 kViewScale3x = 'S300';
const uint32 kViewFullScreen = 'FULL';
const uint32 kViewFiltering = 'SFIL';
const uint32 kViewXBRFiltering = 'SXBR';
const uint32 kViewSmartBorder = 'SBRD';
const uint32 kViewOnScreenKeyboard = 'OKBD';
const uint32 kViewOnScreenMenu = 'OMNU';
const uint32 kDeviceStartStopTape = 'SSTP';
const uint32 kDeviceFastTape = 'FSTP';
const uint32 kDeviceJoystickCursor = 'JCUR';
const uint32 kDeviceJoystickCursorEnter = 'JCEN';
const uint32 kDeviceJoystickKempston = 'JKEM';
const uint32 kDeviceJoystickSinclair2 = 'JSN2';
const uint32 kDeviceJoystickQAOPM = 'JQAM';
const uint32 kDeviceJoystickQAOPSpace = 'JQAS';
const uint32 kDeviceMode48k = 'M48K';
const uint32 kDevicePause = 'PAUS';
const uint32 kDeviceReset = 'REST';
const uint32 kDeviceResetToServiceROM = 'RROM';
const uint32 kDeviceSoundChipAY = 'SCAY';
const uint32 kDeviceSoundChipYM = 'SCYM';
const uint32 kDeviceSoundChipStereoABC = 'SABC';
const uint32 kDeviceSoundChipStereoACB = 'SACB';
const uint32 kDeviceSoundChipStereoBAC = 'SBAC';
const uint32 kDeviceSoundChipStereoBCA = 'SBCA';
const uint32 kDeviceSoundChipStereoCAB = 'SCAB';
const uint32 kDeviceSoundChipStereoCBA = 'SCBA';
const uint32 kDeviceSoundChipStereoMono = 'SMON';
const uint32 kDeviceSoundVolume = 'SVOL';
const uint32 kDeviceBetaDiskA = 'BD_A';
const uint32 kDeviceBetaDiskB = 'BD_B';
const uint32 kDeviceBetaDiskC = 'BD_C';
const uint32 kDeviceBetaDiskD = 'BD_D';
const uint32 kDeviceKempsonMouse = 'KMOU';
const uint32 kHelpRegisterMime = 'RMIM';
const uint32 kHelpOpenHomePage = 'HOME';
const uint32 kPulseEvent = 'TIME';
const uint32 B_SAVE_SCREEN_REQUESTED = 'SSRQ';

#define STATUS_TIME_INFINITE	-1
#define STATUS_TIME_DEFAULT		4

#define SEND_EXTEND_KEY(key, shifted, flags) \
	Handler()->OnKey('c', 0); \
	Handler()->OnKey('s', 0); \
	snooze(25000); \
	Handler()->OnKey(flags & KF_SHIFT ? key : shifted, KF_DOWN | KF_ALT); \
	snooze(25000); \
	Handler()->OnKey(flags & KF_SHIFT ? key : shifted, 0)

namespace xPlatform
{

class HaikuPlatformWindow : public BWindow {
	public:
						HaikuPlatformWindow(BRect frame, int w, int h,
							const char* title, window_type type, uint32 flags);
		virtual			~HaikuPlatformWindow();

		virtual void	MessageReceived(BMessage *msg);
		virtual	bool	QuitRequested();
		virtual void	FrameResized(float width, float height);
		virtual void	WindowActivated(bool active);
		virtual void	Show();

		SurfaceView*	View(void) { return fView; }

	private:
		bool			CheckFuncKey(uint32 key, dword flags);
		byte			TranslateKey(uint32 key, byte last, dword& flags);
		void 			ResizeToScale(int scale);
		void			InstallMimeType(const char *_mime, const char *_icon, const char *_description, const char *_exts);
		void			SetStatusInfo(const char *text, int time = STATUS_TIME_INFINITE, color_which color = B_PANEL_TEXT_COLOR);
		void 			UpdateMenus();
		void			UpdateCursor();

		BMenuBar*		fMenuBar;
		BMenu*			fBetaDiskMenu;
		BMenu*			fSoundChipMenu;
		BMenu*			fSoundVolumeMenu;
		BMenu*			fAYStereoMenu;
		BMenu*			fJoystickMenu;
		BMenu*			fQuickSaveSlotMenu;
		BMenu*			fFilteringMenu;
		BMenuItem*		fFastTapeMenuItem;
		BMenuItem*		fQuickSaveMenuItem;
		BMenuItem*		fVirtualKeyboardMenuItem;
		BMenuItem*		fFilteringMenuItem;
		BMenuItem*		fXBRFilteringMenuItem;
		BMenuItem*		fSmartBorderScaleMenuItem;
		BMenuItem*		fPauseMenuItem;
		BMenuItem*		f48kModeMenuItem;
		BMenuItem*		fResetToServiceROMMenuItem;
		BMenuItem*		fKempstonMouseGrabMenuItem;
		BMenuItem*		fAutoPlayMenuItem;

		BStringView*	fStatusMode;
		BStringView*	fStatusDisk;
		BStringView*	fStatusJoystick;
		BStringView*	fStatusSound;
		BStringView*	fStatusInfo;

		BFilePanel*		fOpenPanel;
		BFilePanel*		fSavePanel;
		BFilePanel*		fSaveScreenPanel;
		
		BMessageRunner* fTimerMessageRunner;

		KeyboardWindow*	fKeyboardWindow;
		WebWindow*		fWebWindow;
		SurfaceView*	fView;

		BRect 			fLastRect;
		bool 			fFullscreen;
		bool			fFiltering;
		bool			fXBRFiltering;
		bool			fKempsonMouseGrab;
		bool			fSkipScale;
		int				fScaleFactor;
		byte			fLastPressedKey;
		int				fStateInfoTimer;
};

}

#endif
