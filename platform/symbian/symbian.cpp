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

#include "../platform.h"

#ifdef _SYMBIAN

#include "../io.h"
#include "../../tools/log.h"
#include "../../ui/ui.h"
#include "../../tools/profiler.h"

#include <eikstart.h>
#include <eikedwin.h>
#include <aknapp.h>
#include <aknappui.h>
#include <akndoc.h>
#include <akncommondialogs.h>
#include <caknfileselectiondialog.h>
#include <avkon.rsg>
#include <aknsoundsystem.h>

#include <unreal_speccy_portable.rsg>
#include "../../build/symbian/unreal_speccy_portable.hrh"

DECLARE_PROFILER_SECTION(dc_draw_symbian);

namespace xPlatform
{

static const char* FileNameToCStr(const TFileName& n)
{
	static char buf[xIo::MAX_PATH_LEN];
    TPtr8 ptr((TUint8*)buf, xIo::MAX_PATH_LEN);
    ptr.Copy(n);
    ptr.ZeroTerminate();
    return buf;
}

void Init()
{
    TFileName appPath;
    CEikonEnv::Static()->FsSession().PrivatePath(appPath);
    appPath.Insert(0, CEikonEnv::Static()->EikAppUi()->Application()->AppFullName().Left(2));
    xIo::SetResourcePath(FileNameToCStr(appPath));
    xLog::SetLogPath("e:\\usp\\");
    Handler()->OnInit();
}
void Done()
{
    Handler()->OnDone();
}

class TDCControl : public CCoeControl, MCoeControlObserver
{
public:
	void ConstructL(const TRect& aRect);
	TDCControl()
		: iPeriodic(NULL), bitmap(NULL), frame(0), key_flags(KF_CURSOR|KF_KEMPSTON)
		{}
	virtual ~TDCControl();

	void Reset() { Handler()->OnAction(A_RESET); }
	void OpenFile();
	void ToggleTape() { Handler()->OnAction(A_TAPE_TOGGLE); }

public:
	static TInt TimerCallBack(TAny* aInstance);

private:
	void OnTimer();
	void Update() const;
	void HandleResourceChange(TInt aType);
	TInt CountComponentControls() const { return 0; }
	CCoeControl* ComponentControl(TInt aIndex) const { return NULL; }
	void Draw(const TRect& aRect) const;
	void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType) {}
	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
protected:
	CPeriodic* iPeriodic;
	CFbsBitmap* bitmap;
	int frame;

	struct eMouse
	{
		enum eDir { D_NONE = 0x00, D_UP = 0x01, D_DOWN = 0x02, D_LEFT = 0x04, D_RIGHT = 0x08 };
		eMouse() : enable(true), dir(D_NONE), x(0), y(0) {}
		bool enable;
		byte dir;
		byte x, y;
		bool Update();
	};
	mutable eMouse mouse;
	dword key_flags;

	dword color_cache[16];
};
bool TDCControl::eMouse::Update()
{
	if(dir&D_UP)		y += 1;
	else if(dir&D_DOWN)	y -= 1;
	else y = 0;
	if(dir&D_LEFT)		x -= 1;
	else if(dir&D_RIGHT)x += 1;
	else x = 0;

	return dir != D_NONE;
}

static inline dword BGRX(byte r, byte g, byte b)
{
	return (r << 16)|(g << 8)|b;
}

void TDCControl::ConstructL(const TRect& /*aRect*/)
{
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

	bitmap = new CFbsBitmap;//(iEikonEnv->WsSession());
	bitmap->Create(TSize(320, 240), EColor16MU);
	iPeriodic = CPeriodic::NewL( CActive::EPriorityStandard );
	// 50 fps 20000 mks interval
	iPeriodic->Start(20000, 20000, TCallBack( TDCControl::TimerCallBack, this));
}
TDCControl::~TDCControl()
{
	delete iPeriodic;
	delete bitmap;
	Done();
}
void TDCControl::Draw(const TRect& /*aRect*/) const
{
	CWindowGc& gc = SystemGc();
	gc.SetBrushColor(0);
	gc.Clear(Rect());
	if(bitmap)
	{
		Update();
		gc.BitBlt(TPoint(0, 0), bitmap);
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

void TDCControl::Update() const
{
	if(mouse.enable && mouse.Update())
	{
		Handler()->OnMouse(MA_MOVE, mouse.x, mouse.y);
	}
	Handler()->OnLoop();

	PROFILER_BEGIN(dc_draw_symbian);
	byte* data = (byte*)Handler()->VideoData();
	dword* data_ui = (dword*)Handler()->VideoDataUI();
	bitmap->LockHeap();
	dword* tex = (dword*)bitmap->DataAddress();

	if(data_ui)
	{
		for(int i = 0; i < 320*240; ++i)
		{
			xUi::eRGBAColor c_ui = *data_ui++;
			xUi::eRGBAColor c = color_cache[*data++];
			*tex++ = BGRX((c.b >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.r >> c_ui.a) + c_ui.b);
		}
	}
	else
	{
		for(int i = 0; i < 320*240; ++i)
		{
			*tex++ = color_cache[*data++];
		}
	}
	bitmap->UnlockHeap();
	PROFILER_END(dc_draw_symbian);
}
void TDCControl::OnTimer()
{
	++frame;
	DrawDeferred();

	if(!(frame%100))
		User::ResetInactivityTime();
	if(!(frame%50))
		User::After(0);
}
TInt TDCControl::TimerCallBack( TAny* aInstance )
{
	((TDCControl*)aInstance)->OnTimer();
	return 0;
}
static char TranslateKey(const TKeyEvent& aKeyEvent)
{
    switch(aKeyEvent.iScanCode)
    {
    case '5':
    case EStdKeyEnter:
    case EStdKeyDevice3:		return 'f';
    case '4':
    case EStdKeyLeftArrow:		return 'l';
    case '6':
    case EStdKeyRightArrow:		return 'r';
    case '2':
    case EStdKeyUpArrow:		return 'u';
    case '8':
    case EStdKeyDownArrow:      return 'd';
    case EStdKeyHash:			return ' ';
    case '0':					return 'e';
    case '1':					return '`';
    case '*':					return '\\';
    default : break;
    }
    return 0;
}
TKeyResponse TDCControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
    char ch = TranslateKey(aKeyEvent);
    if(!ch)
        return EKeyWasNotConsumed;
    switch(aType)
    {
    case EEventKeyDown:
        Handler()->OnKey(ch, KF_DOWN|key_flags);
        switch(ch)
        {
        case 'u':   mouse.dir |= eMouse::D_UP; break;
        case 'd':   mouse.dir |= eMouse::D_DOWN; break;
        case 'l':   mouse.dir |= eMouse::D_LEFT; break;
        case 'r':   mouse.dir |= eMouse::D_RIGHT; break;
        case 'f':   Handler()->OnMouse(MA_BUTTON, 0, 1); break;
        default : break;
        }
        break;
    case EEventKeyUp:
        Handler()->OnKey(ch, key_flags);
        switch(ch)
        {
        case 'u':   mouse.dir &= ~eMouse::D_UP; break;
        case 'd':   mouse.dir &= ~eMouse::D_DOWN; break;
        case 'l':   mouse.dir &= ~eMouse::D_LEFT; break;
        case 'r':   mouse.dir &= ~eMouse::D_RIGHT; break;
        case 'f':   Handler()->OnMouse(MA_BUTTON, 0, 0); break;
        default : break;
        }
        break;
    default:
        break;
    }
    return EKeyWasConsumed;
}
void TDCControl::OpenFile()
{
	TFileName openFileName;
	if(AknCommonDialogs::RunSelectDlgLD(openFileName, R_FILE_SELECTION_DIALOG))
	{
		Handler()->OnOpenFile(FileNameToCStr(openFileName));
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
	TUid AppDllUid() const { const TUid KUidSimpleCube = { 0xA000821A }; return KUidSimpleCube; }
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
