/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef _SYMBIAN

#include "../io.h"
#include "../../tools/log.h"
#include "../../ui/ui.h"
#include "../../tools/profiler.h"
#include "../../tools/options.h"
#include "../../tools/tick.h"
#include "../../options_common.h"

#include <eikstart.h>
#include <eikedwin.h>
#include <aknapp.h>
#include <aknappui.h>
#include <akndoc.h>
#include <akncommondialogs.h>
#include <caknfileselectiondialog.h>
#include <caknmemoryselectiondialog.h>
#include <pathinfo.h>
#include <avkon.rsg>
#include <aknsoundsystem.h>
#include <remconcoreapitargetobserver.h>
#include <remconcoreapitarget.h>
#include <remconinterfaceselector.h>
#include <utf.h>

#include <unreal_speccy_portable.rsg>
#include "../../build/symbian/unreal_speccy_portable.hrh"

PROFILER_DECLARE(draw);
PROFILER_DECLARE(blit);
PROFILER_DECLARE(sound);

namespace xPlatform
{

static struct eOptionSkipFrames : public xOptions::eOptionInt
{
	eOptionSkipFrames() { Set(2); }
	virtual const char* Name() const { return "skip frames"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "off", "1", "2", "4", "8", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(0, 5, next);
	}
	int Values(int id) const { static const int vals[] = { 0, 1, 2, 4, 8 }; return vals[id]; }
	virtual int Order() const { return 1; }
} op_skip_frames;

static struct eOptionRotateScreen : public xOptions::eOptionBool
{
	eOptionRotateScreen() { Set(false); }
	virtual const char* Name() const { return "rotate screen"; }
	virtual int Order() const { return 2; }
} op_rotate_screen;

static struct eOptionRotateJoy : public xOptions::eOptionBool
{
	eOptionRotateJoy() { Set(true); }
	virtual const char* Name() const { return "rotate joystick"; }
	virtual int Order() const { return 3; }
} op_rotate_joystick;

static struct eOptionKeyboard : public xOptions::eOptionInt
{
	enum eType { KT_FIRST, KT_NUMPAD = KT_FIRST, KT_COMBINED, KT_FULL, KT_LAST };
	eOptionKeyboard() { Set(KT_NUMPAD); }
	virtual const char* Name() const { return "keyboard"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "numpad", "combined", "full", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(KT_FIRST, KT_LAST, next);
	}
	virtual int Order() const { return 4; }
} op_keyboard;

static struct eOptionNoBorder : public xOptions::eOptionBool
{
	eOptionNoBorder() { Set(false); }
	virtual const char* Name() const { return "no border"; }
	virtual int Order() const { return 5; }
} op_no_border;


void InitSound();
void DoneSound();
void OnLoopSound();

static const char* FileNameToCStr(const TFileName& n)
{
	static char buf[xIo::MAX_PATH_LEN];
    TPtr8 ptr((TUint8*)buf, xIo::MAX_PATH_LEN);
    CnvUtfConverter::ConvertFromUnicodeToUtf8(ptr, n);
    ptr.SetLength(n.Length());
    ptr.ZeroTerminate();
    return buf;
}
static void CStrToFileName(const char* n, TFileName* name)
{
	int l = strlen(n);
	TPtrC8 ptr((TUint8*)n, l + 1);
	CnvUtfConverter::ConvertToUnicodeFromUtf8(*name, ptr);
    name->SetLength(l);
    name->ZeroTerminate();
}

void Init()
{
    TFileName appPath;
    CEikonEnv::Static()->FsSession().PrivatePath(appPath);
    appPath.Insert(0, CEikonEnv::Static()->EikAppUi()->Application()->AppFullName().Left(2));
    const char* p = FileNameToCStr(appPath);
    xIo::SetResourcePath(p);
    xIo::SetProfilePath(p);
    xLog::SetLogPath("e:\\");
    Handler()->OnInit();
    InitSound();
}
void Done()
{
    DoneSound();
    Handler()->OnDone();
}

class TDCControl : public CCoeControl, MCoeControlObserver, MRemConCoreApiTargetObserver
{
public:
	void ConstructL(const TRect& aRect);
	TDCControl() : iTimer(NULL), bitmap(NULL), frame(0)	{}
	virtual ~TDCControl();

	void Reset() { Handler()->OnAction(A_RESET); }
	void OpenFile();
	void ToggleTape() { Handler()->OnAction(A_TAPE_TOGGLE); }

public:
	static TInt TimerCallBack(TAny* aInstance);

private:
	void OnTimer();
	void Draw(bool horizontal) const;
	void HandleResourceChange(TInt aType);
	TInt CountComponentControls() const { return 0; }
	CCoeControl* ComponentControl(TInt aIndex) const { return NULL; }
	void Draw(const TRect& aRect) const;
	void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType) {}
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
	virtual void MrccatoCommand(TRemConCoreApiOperationId, TRemConCoreApiButtonAction);
protected:
	CIdle* iTimer;
	CFbsBitmap* bitmap;
	mutable int frame;
	xOptions::eOption<int>* op_volume;
	CRemConInterfaceSelector* iInterfaceSelector;
    CRemConCoreApiTarget*     iCoreTarget;

//	struct eMouse
//	{
//		enum eDir { D_NONE = 0x00, D_UP = 0x01, D_DOWN = 0x02, D_LEFT = 0x04, D_RIGHT = 0x08 };
//		eMouse() : enable(false), dir(D_NONE), x(0), y(0) {}
//		bool enable;
//		byte dir;
//		byte x, y;
//		bool Update();
//	};
//	mutable eMouse mouse;

	dword color_cache[16];
	eTick tick;
};
//bool TDCControl::eMouse::Update()
//{
//	if(dir&D_UP)		y += 1;
//	else if(dir&D_DOWN)	y -= 1;
//	else y = 0;
//	if(dir&D_LEFT)		x -= 1;
//	else if(dir&D_RIGHT)x += 1;
//	else x = 0;
//
//	return dir != D_NONE;
//}

static inline dword BGRX(byte r, byte g, byte b)
{
	return (r << 16)|(g << 8)|b;
}

void TDCControl::ConstructL(const TRect& /*aRect*/)
{
	op_volume = xOptions::eOption<int>::Find("volume");
	iInterfaceSelector = CRemConInterfaceSelector::NewL();
	iCoreTarget = CRemConCoreApiTarget::NewL(*iInterfaceSelector, *this);
	iInterfaceSelector->OpenTargetL();

	CreateWindowL();
	SetExtentToWholeScreen();
	ActivateL();
	Init();

	const byte brightness = 200;
	const byte bright_intensity = 55;
	for(int c = 0; c < 16; ++c)
	{
		byte i = c&8 ? brightness + bright_intensity : brightness;
		byte b = c&1 ? i : 0;
		byte r = c&2 ? i : 0;
		byte g = c&4 ? i : 0;
		color_cache[c] = BGRX(r, g ,b);
	}

	bitmap = new CFbsBitmap;
	bitmap->Create(TSize(320, 320), EColor16MU);
	iTimer = CIdle::NewL(CActive::EPriorityIdle);
	iTimer->Start(TCallBack(TDCControl::TimerCallBack, this));
	tick.SetCurrent();
}
TDCControl::~TDCControl()
{
	SAFE_CALL(iTimer)->Cancel();
	SAFE_DELETE(iTimer);
	SAFE_DELETE(bitmap);
	Done();
}
void TDCControl::Draw(const TRect& /*aRect*/) const
{
	CWindowGc& gc = SystemGc();
	gc.SetBrushColor(0);
	TRect r = Rect();
	gc.Clear(r);
	if(bitmap)
	{
		bool h = (r.Width() > r.Height()) ^ op_rotate_screen;
		Draw(h);
		PROFILER_SECTION(blit);
		TRect rb(0, 0, h ? 320 : 240, h ? 240 : 320);
		int dx = r.Width() - rb.Width();
		int dy = r.Height() - rb.Height();
		if(dx < 0)	dx = 0;
		if(dy < 0)	dy = 0;
		gc.BitBlt(TPoint(dx/2, dy/2), bitmap, rb);
	}
}

#define BLEND_UI_PIXELH(op) \
c_ui = *data_ui++; \
c = color_cache[*op]; \
*tex++ = BGRX((c.b >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.r >> c_ui.a) + c_ui.b); \

#define BLEND_UI_LINEH \
for(int x = 0; x < 64; ++x) \
{ \
	BLEND_UI_PIXELH(data++); \
	BLEND_UI_PIXELH(data++); \
	BLEND_UI_PIXELH(data++); \
	BLEND_UI_PIXELH(data); \
	BLEND_UI_PIXELH(data++); \
} \
data += 64; \

#define COPY_LINEH \
for(int x = 0; x < 64; ++x) \
{ \
	*tex++ = color_cache[*data++]; \
	*tex++ = color_cache[*data++]; \
	*tex++ = color_cache[*data++]; \
	*tex++ = color_cache[*data]; \
	*tex++ = color_cache[*data++]; \
} \
data += 64; \

void TDCControl::Draw(bool horizontal) const
{
	PROFILER_SECTION(draw);
	byte* data = (byte*)Handler()->VideoData();
	dword* data_ui = (dword*)Handler()->VideoDataUI();
	bitmap->LockHeap();
	dword* tex = (dword*)bitmap->DataAddress();

	if(horizontal)
	{
		if(op_no_border)
		{
			data += 24*320 + 32; // top left corner without border
			if(data_ui)
			{
				xUi::eRGBAColor c_ui;
				xUi::eRGBAColor c;
				for(int y = 0; y < 48; ++y)
				{
					for(int y1 = 0; y1 < 4; ++y1)
					{
						BLEND_UI_LINEH;
					}
					data -= 320;
					BLEND_UI_LINEH;
				}
			}
			else
			{
				for(int y = 0; y < 48; ++y)
				{
					for(int y1 = 0; y1 < 4; ++y1)
					{
						COPY_LINEH;
					}
					data -= 320;
					COPY_LINEH;
				}
			}
		}
		else
		{
			if(data_ui)
			{
				xUi::eRGBAColor c_ui;
				xUi::eRGBAColor c;
				for(int i = 0; i < 320*240; ++i)
				{
					BLEND_UI_PIXELH(data++);
				}
			}
			else
			{
				for(int i = 0; i < 320*240; ++i)
				{
					*tex++ = color_cache[*data++];
				}
			}
		}
	}
	else
	{
		if(data_ui)
		{
			for(int j = 0; j < 320; j++)
			{
				byte* d = data + 320*239 + j;
				dword* d_ui = data_ui + 320*239 + j;
				for(int i = 0; i < 240; ++i)
				{
					xUi::eRGBAColor c_ui = *d_ui;
					xUi::eRGBAColor c = color_cache[*d];
					*tex++ = BGRX((c.b >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.r >> c_ui.a) + c_ui.b);
					d -= 320;
					d_ui -= 320;
				}
				tex += 320-240;
			}
		}
		else
		{
			for(int j = 0; j < 320; j++)
			{
				byte* d = data + 320*239 + j;
				for(int i = 0; i < 240; ++i)
				{
					*tex++ = color_cache[*d];
					d -= 320;
				}
				tex += 320-240;
			}
		}
	}
	bitmap->UnlockHeap();
}
void TDCControl::OnTimer()
{
	if(OpQuit())
		CEikonEnv::Static()->EikAppUi()->HandleCommandL(EEikCmdExit);
//	if(mouse.enable && mouse.Update())
//	{
//		Handler()->OnMouse(MA_MOVE, mouse.x, mouse.y);
//	}
	Handler()->OnLoop();

	++frame;
	if(!(frame%100))
		User::ResetInactivityTime();
	int sf = op_skip_frames.Values(op_skip_frames) + 1;
	if(frame % sf == 0)
	{
		DrawDeferred();
		qword mks_org = 20000*sf;
		qword mks = tick.Passed().Mks();
		if(mks < mks_org && !Handler()->FullSpeed())
		{
			User::AfterHighRes(mks_org - mks);
		}
		tick.SetCurrent();
	}
	{
		PROFILER_SECTION(sound);
		OnLoopSound();
	}
}
void TDCControl::HandleResourceChange(TInt aType)
{
	switch(aType)
	{
	case KEikDynamicLayoutVariantSwitch:
		SetExtentToWholeScreen();
		break;
	}
}
TInt TDCControl::TimerCallBack( TAny* aInstance )
{
	((TDCControl*)aInstance)->OnTimer();
	return 1;
}
static char TranslateKey(const TKeyEvent& aKeyEvent, dword& flags)
{
	eOptionKeyboard::eType keyboard = (eOptionKeyboard::eType)(int)op_keyboard;
	if(aKeyEvent.iModifiers&EModifierShift)
		flags |= KF_SHIFT;
	if(aKeyEvent.iModifiers&EModifierCtrl)
		flags |= KF_ALT;
	if(keyboard == eOptionKeyboard::KT_COMBINED)
	{
		bool num_mod = (aKeyEvent.iModifiers&EModifierFunc) != 0;
		if(num_mod && aKeyEvent.iScanCode >= '0' && aKeyEvent.iScanCode <= '9')
	    	return aKeyEvent.iScanCode;
		switch(aKeyEvent.iScanCode)
		{
		case '1':				return 'R';
		case '2':				return 'T';
		case '3':				return 'Y';
	    case '*':				return 'U';
		case '4':				return 'F';
		case '5':				return 'G';
		case '6':				return 'H';
	    case EStdKeyHash:		return 'J';
		case '7':				return 'V';
		case '8':				return 'B';
		case '9':				return 'N';
		case '0':				return 'M';
		}
	}
	if(keyboard == eOptionKeyboard::KT_FULL)
	{
		if(aKeyEvent.iScanCode >= '0' && aKeyEvent.iScanCode <= '9')
	    	return aKeyEvent.iScanCode;
	}
	if(keyboard != eOptionKeyboard::KT_NUMPAD)
	{
	    if(aKeyEvent.iScanCode >= 'A' && aKeyEvent.iScanCode <= 'Z')
	    	return aKeyEvent.iScanCode;
		switch(aKeyEvent.iScanCode)
		{
		case EStdKeyLeftFunc:		return 'k';
		case EStdKeyEnter:			return 'e';
		case EStdKeyLeftShift:
		case EStdKeyRightShift:		return 'c';
		case EStdKeySpace:			return ' ';
		}
	}
	bool rotate = op_rotate_joystick;
    switch(aKeyEvent.iScanCode)
    {
    case '5':
    case EStdKeyDevice3:		return 'f';
    case '4':
    case EStdKeyLeftArrow:		return rotate ? 'd' : 'l';
    case '6':
    case EStdKeyRightArrow:		return rotate ? 'u' : 'r';
    case '2':
    case EStdKeyUpArrow:		return rotate ? 'l' : 'u';
    case '8':
    case EStdKeyDownArrow:      return rotate ? 'r' : 'd';
    case EStdKeyHash:			return ' ';
    case '0':					return 'e';
    case '*':					return 'k';
    case '3':					return 'p';
    case EStdKeyYes:			return 'm';
	case EStdKeyBackspace:
		flags |= KF_SHIFT;
		return '0';
    }
    return 0;
}
TKeyResponse TDCControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
	dword flags = OpJoyKeyFlags();
	switch(aType)
	{
	case EEventKeyDown:
		flags |= KF_DOWN;
		break;
	case EEventKeyUp:
		break;
	default:
		return EKeyWasNotConsumed;
	}
	char ch = TranslateKey(aKeyEvent, flags);
	if(!ch)
		return EKeyWasNotConsumed;
	Handler()->OnKey(ch, flags);
	return EKeyWasConsumed;
}
void TDCControl::MrccatoCommand(TRemConCoreApiOperationId id, TRemConCoreApiButtonAction a)
{
	if(op_volume && a == ERemConCoreApiButtonClick)
	{
		switch(id)
		{
		case ERemConCoreApiVolumeUp:
			if(*op_volume < xPlatform::V_100)
				op_volume->Change(true);
			break;
		case ERemConCoreApiVolumeDown:
			if(*op_volume > xPlatform::V_MUTE)
				op_volume->Change(false);
			break;
		default: break;
		}
	}
}
void TDCControl::OpenFile()
{
	TFileName path;
	CStrToFileName(OpLastFolder(), &path);
	for(;;)
	{
		if(!path.Length())
		{
			CAknMemorySelectionDialog* memDlg = CAknMemorySelectionDialog::NewL(ECFDDialogTypeSelect, ETrue);
			CAknMemorySelectionDialog::TMemory memory = CAknMemorySelectionDialog::EPhoneMemory;
			bool cancel = memDlg->ExecuteL(memory) == CAknFileSelectionDialog::ERightSoftkey;
			delete memDlg;
			if(cancel)
				break;
			if(memory==CAknMemorySelectionDialog::EMemoryCard)
				path = PathInfo::MemoryCardRootPath();
			else
				path = PathInfo::PhoneMemoryRootPath();
		}
		CAknFileSelectionDialog* fileDlg = CAknFileSelectionDialog::NewL(ECFDDialogTypeSelect, R_FILE_SELECTION_DIALOG);
		bool ok = fileDlg->ExecuteL(path);
		delete fileDlg;
		if(ok)
		{
			const char* name = FileNameToCStr(path);
			Handler()->OnOpenFile(name);
			break;
		}
		path.Zero();
	}
}


class TAppUi : public CAknAppUi
{
public:
	void ConstructL()
	{
		BaseConstructL();
		gl_control = new (ELeave) TDCControl;
		gl_control->SetMopParent(this);
		gl_control->ConstructL(ClientRect());
		AddToStackL( gl_control );
		SetKeyBlockMode(ENoKeyBlock);
		CAknKeySoundSystem* ks = KeySounds();
		if(ks)
		{
			ks->PushContextL(R_AVKON_SILENT_SKEY_LIST);
			ks->BringToForeground();
			ks->LockContext();
		}
	}
	virtual ~TAppUi()
	{
		if(gl_control)
		{
			RemoveFromStack(gl_control);
			delete gl_control;
		}
		CAknKeySoundSystem* ks = KeySounds();
		if(ks)
		{
			ks->ReleaseContext();
			ks->PopContext();
		}
	}

private:
	void HandleCommandL(TInt aCommand);
	virtual TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
		return EKeyWasNotConsumed;
	}

private:
	TDCControl* gl_control;
};

void TAppUi::HandleCommandL(TInt aCommand)
{
	switch(aCommand)
	{
	case EEikCmdExit:
	case EAknSoftkeyExit:
		Exit();
		break;
	case EReset:
		gl_control->Reset();
		break;
	case EOpenFile:
		gl_control->OpenFile();
		break;
	case EToggleTape:
		gl_control->ToggleTape();
	default:
		break;
	}
}

class TDoc : public CAknDocument
{
public:
	TDoc(CEikApplication& aApp) : CAknDocument(aApp)	{}
	virtual ~TDoc() {}

private:
	CEikAppUi* CreateAppUiL() { return new (ELeave) TAppUi; }
};

class TApp : public CAknApplication
{
private:
	CApaDocument* CreateDocumentL()	{ return new (ELeave) TDoc(*this); }
	TUid AppDllUid() const { const TUid KUid = { 0xA89FAC37 }; return KUid; }
};

LOCAL_C CApaApplication* NewApplication()
{
	return new TApp;
}

}
//namespace xPlatform

GLDEF_C TInt E32Main()
{
	return EikStart::RunApplication(xPlatform::NewApplication);
}

#endif//_SYMBIAN
