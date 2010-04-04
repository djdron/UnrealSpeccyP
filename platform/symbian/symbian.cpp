#include "../../std.h"

#ifdef _SYMBIAN

#include "../platform.h"
#include "../io.h"
#include "../log.h"

#include <eikstart.h>
#include <aknapp.h>
#include <aknappui.h>
#include <akndoc.h>
#include <akncommondialogs.h>
#include <caknfileselectiondialog.h>

#include <unreal_speccy_portable.rsg>
#include "../../symbian/unreal_speccy_portable.hrh"

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
	TDCControl() : frame(0) {}
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
		eMouse() : dir(D_NONE), x(0), y(0) {}
		byte dir;
		byte x, y;
		bool Update();
	};
	mutable eMouse mouse;
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
void TDCControl::ConstructL(const TRect& /*aRect*/)
{
	CreateWindowL();
	SetExtentToWholeScreen();
	ActivateL();
    Init();
	bitmap = new CFbsBitmap;//(iEikonEnv->WsSession());
	bitmap->Create(TSize(320, 240), EColor16MU);
	iPeriodic = CPeriodic::NewL( CActive::EPriorityIdle );
	iPeriodic->Start(100, 100, TCallBack( TDCControl::TimerCallBack, this));
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
	switch( aType )
	{
	case KEikDynamicLayoutVariantSwitch:
		SetExtentToWholeScreen();
		break;
	}
}
#define BGRX(r, g, b) ((r << 16)|(g << 8)|b)

void TDCControl::Update() const
{
	if(mouse.Update())
	{
		Handler()->OnMouse(MA_MOVE, mouse.x, mouse.y);
	}
	Handler()->OnLoop();
	byte* data = (byte*)Handler()->VideoData();

	bitmap->LockHeap();
	dword* tex = (dword*)bitmap->DataAddress();
    const byte brightness = 200;
    const byte bright_intensity = 55;
	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			byte r, g, b;
			byte c = data[y*320 + x];
			byte i = c&8 ? brightness + bright_intensity : brightness;
			b = c&1 ? i : 0;
			r = c&2 ? i : 0;
			g = c&4 ? i : 0;
			dword* p = &tex[y*320 + x];
			*p = BGRX(r, g ,b);
		}
	}
	bitmap->UnlockHeap();
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
    case EStdKeyEnter:
    case EStdKeyDevice3:		return 'e';
    case EStdKeyLeftArrow:		return 'l';
    case EStdKeyRightArrow:		return 'r';
    case EStdKeyUpArrow:		return 'u';
    case EStdKeyDownArrow:      return 'd';
    case EStdKeyHash:			return ' ';
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
    case EEventKey:
        Handler()->OnKey(ch, KF_DOWN);
        switch(ch)
        {
        case 'u':   mouse.dir |= eMouse::D_UP; break;
        case 'd':   mouse.dir |= eMouse::D_DOWN; break;
        case 'l':   mouse.dir |= eMouse::D_LEFT; break;
        case 'r':   mouse.dir |= eMouse::D_RIGHT; break;
        case 'e':   Handler()->OnMouse(MA_BUTTON, 0, 1); break;
        default : break;
        }
        break;
    case EEventKeyUp:
        Handler()->OnKey(ch, 0);
        switch(ch)
        {
        case 'u':   mouse.dir &= ~eMouse::D_UP; break;
        case 'd':   mouse.dir &= ~eMouse::D_DOWN; break;
        case 'l':   mouse.dir &= ~eMouse::D_LEFT; break;
        case 'r':   mouse.dir &= ~eMouse::D_RIGHT; break;
        case 'e':   Handler()->OnMouse(MA_BUTTON, 0, 0); break;
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
	}
	virtual ~TAppUi()
	{
		if(gl_control)
		{
			RemoveFromStack(gl_control);
			delete gl_control;
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
	case EAknSoftkeyBack:
	case EEikCmdExit:
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
