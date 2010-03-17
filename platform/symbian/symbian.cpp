#include "../../std.h"

#ifdef _SYMBIAN

#include "../platform.h"
#include "../io.h"

#include <eikstart.h>
#include <aknapp.h>
#include <aknappui.h>
#include <GLES/egl.h>
#include <akndoc.h>

#include <unreal_speccy_portable.rsg>
#include "../../symbian/unreal_speccy_portable.hrh"

namespace xPlatform
{

void Init()
{
    xIo::SetResourcePath("e:\\usp\\");
    Handler()->OnInit();
}
void Done()
{
    Handler()->OnDone();
}

class TGLScene : public CBase
{
public:
	TGLScene(TUint _width, TUint _height) { SetScreenSize(_width, _height); }
	virtual ~TGLScene() {}

public:
	void AppInit();
	void AppExit();
	void AppCycle(TInt aFrame);
	void SetScreenSize(TUint _width, TUint _height) { width = _width; height = _height; }
private:
	TUint width;
	TUint height;
};

/** Vertex coordinates */
static const GLbyte vertices[4 * 2] =
{
	0, 0,
	0, 1,
	1, 1,
	1, 0,
};

static const GLubyte uvs[4 * 2] =
{
    0, 0,
    0, 1,
    1, 1,
    1, 0,
};

/** Colors for vertices (Red, Green, Blue, Alpha). */
//static const GLubyte colors[4 * 4] =
//{
//	255 ,255, 255, 255,
//	255 ,255, 255, 255,
//	255 ,255, 255, 255,
//	255 ,255, 255, 255,
//};

/**
 * Indices for drawing the triangles.
 * The color of the triangle is determined by
 * the color of the last vertex of the triangle.
 */
static const GLubyte triangles[2 * 3] =
{
	0,1,2,
	0,2,3,
};

void TGLScene::AppInit( void )
{
    Init();
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_BYTE, 0, vertices);
//	glEnableClientState(GL_COLOR_ARRAY);
//	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_BYTE, 0, uvs);
    glShadeModel(GL_FLAT);
}
void TGLScene::AppExit()
{
    Done();
}

#define RGBX(r, g, b)	((b << 16)|(g << 8)|r)

static dword tex[512*256];
void TGLScene::AppCycle(TInt aFrame)
{
    Handler()->OnLoop();

    const byte brightness = 200;
    const byte bright_intensity = 55;

	byte* data = (byte*)Handler()->VideoData();
	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for(int y = 0; y < 240; ++y)
	{
		for(int x = 0; x < 320; ++x)
		{
			byte r, g, b;
			byte c = data[y*320+x];
			byte i = c&8 ? brightness + bright_intensity : brightness;
			b = c&1 ? i : 0;
			r = c&2 ? i : 0;
			g = c&4 ? i : 0;
			dword* p = &tex[y*512+x];
			*p++ = RGBX(r, g ,b);
		}
	}
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(320.0f/512.0f, 240.0f/256.0f, 1.0f);

	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthof(0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_BYTE, triangles);
//	glFlush();
}

class TGLControl : public CCoeControl, MCoeControlObserver
{
public:
	void ConstructL(const TRect& aRect);
	virtual ~TGLControl();

public:
	static TInt DrawCallBack(TAny* aInstance);

private:
	void SizeChanged();
	void HandleResourceChange(TInt aType);
	TInt CountComponentControls() const { return 0; }
	CCoeControl* ComponentControl(TInt aIndex) const { return NULL; }
	void Draw(const TRect& aRect) const;
	void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType) {}

private:
	EGLDisplay iEglDisplay;
	EGLSurface iEglSurface;
	EGLContext iEglContext;
	CPeriodic* iPeriodic;
	TBool iOpenGlInitialized;
	TInt iFrame;
public:
	TGLScene* scene;
};

void TGLControl::ConstructL(const TRect& /*aRect*/)
{
	iOpenGlInitialized = EFalse;
	CreateWindowL();

	SetExtentToWholeScreen();
	ActivateL();

	iFrame = 0;

	EGLConfig Config;
	iEglDisplay = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	if ( iEglDisplay == NULL )
	{
		_LIT(KGetDisplayFailed, "eglGetDisplay failed");
		User::Panic( KGetDisplayFailed, 0 );
	}
	if ( eglInitialize( iEglDisplay, NULL, NULL ) == EGL_FALSE )
	{
		_LIT(KInitializeFailed, "eglInitialize failed");
		User::Panic( KInitializeFailed, 0 );
	}

	EGLConfig *configList = NULL; // Pointer for EGLConfigs
	EGLint numOfConfigs = 0;
	EGLint configSize = 0;

	if ( eglGetConfigs( iEglDisplay, configList, configSize, &numOfConfigs )
			== EGL_FALSE )
	{
		_LIT(KGetConfigsFailed, "eglGetConfigs failed");
		User::Panic( KGetConfigsFailed, 0 );
	}

	configSize = numOfConfigs;

	configList = (EGLConfig*) User::Alloc( sizeof(EGLConfig)*configSize );
	if ( configList == NULL )
	{
		_LIT(KConfigAllocFailed, "config alloc failed");
		User::Panic( KConfigAllocFailed, 0 );
	}

	TDisplayMode DMode = Window().DisplayMode();
	TInt BufferSize = 0;

	switch(DMode)
	{
	case EColor4K:
		BufferSize = 12;
        break;
	case EColor64K:
		BufferSize = 16;
		break;
	case EColor16M:
		BufferSize = 24;
		break;
	case EColor16MU:
		BufferSize = 32;
		break;
	default:
		_LIT(KDModeError, "unsupported displaymode");
		User::Panic( KDModeError, 0 );
		break;
	}

	const EGLint attrib_list[] = {	EGL_BUFFER_SIZE, BufferSize, EGL_NONE };

	if(eglChooseConfig( iEglDisplay, attrib_list, configList, configSize, &numOfConfigs ) == EGL_FALSE )
	{
		_LIT(KChooseConfigFailed, "eglChooseConfig failed");
		User::Panic( KChooseConfigFailed, 0 );
	}

	Config = configList[0];
	User::Free( configList );

	iEglSurface = eglCreateWindowSurface( iEglDisplay, Config, &Window(), NULL );
	if(iEglSurface == NULL)
	{
		_LIT(KCreateWindowSurfaceFailed, "eglCreateWindowSurface failed");
		User::Panic( KCreateWindowSurfaceFailed, 0 );
	}

	iEglContext = eglCreateContext( iEglDisplay, Config, EGL_NO_CONTEXT, NULL );
	if(iEglContext == NULL)
	{
		_LIT(KCreateContextFailed, "eglCreateContext failed");
		User::Panic( KCreateContextFailed, 0 );
	}

	if(eglMakeCurrent( iEglDisplay, iEglSurface, iEglSurface, iEglContext ) == EGL_FALSE )
	{
		_LIT(KMakeCurrentFailed, "eglMakeCurrent failed");
		User::Panic( KMakeCurrentFailed, 0 );
	}

	TSize size;
	size = this->Size();

	scene = new (ELeave) TGLScene(size.iWidth, size.iHeight);
	scene->AppInit();

	iOpenGlInitialized = ETrue;

	iPeriodic = CPeriodic::NewL( CActive::EPriorityIdle );
	iPeriodic->Start( 100, 100, TCallBack( TGLControl::DrawCallBack, this ) );
}

TGLControl::~TGLControl()
{
	delete iPeriodic;

	if ( scene )
	{
		scene->AppExit();
		delete scene;
	}

	eglMakeCurrent( iEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroySurface( iEglDisplay, iEglSurface );
	eglDestroyContext( iEglDisplay, iEglContext );
	eglTerminate( iEglDisplay );
}

void TGLControl::SizeChanged()
{
	if( iOpenGlInitialized && scene )
	{
		TSize size;
		size = this->Size();

		scene->SetScreenSize( size.iWidth, size.iHeight );
	}
}

void TGLControl::HandleResourceChange(TInt aType)
{
	switch( aType )
	{
		case KEikDynamicLayoutVariantSwitch:
		SetExtentToWholeScreen();
		break;
	}
}

void TGLControl::Draw(const TRect& /*aRect*/) const
{
	CWindowGc& gc = SystemGc();
	gc.Clear( Rect() );
}

TInt TGLControl::DrawCallBack( TAny* aInstance )
{
	TGLControl* instance = (TGLControl*) aInstance;
	instance->iFrame++;

	instance->scene->AppCycle(instance->iFrame);

	eglSwapBuffers(instance->iEglDisplay, instance->iEglSurface);

	if(!(instance->iFrame%100))
	{
		User::ResetInactivityTime();
	}

	if(!(instance->iFrame%50))
	{
		User::After(0);
	}

	return 0;
}

class TAppUi : public CAknAppUi
{
public:
	void ConstructL()
	{
		BaseConstructL();
		gl_control = new (ELeave) TGLControl;
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
	TGLControl* gl_control;
};

void TAppUi::HandleCommandL(TInt aCommand)
{
	switch(aCommand)
	{
	case EAknSoftkeyBack:
	case EEikCmdExit:
		Exit();
		break;
	case ESimpleCubeFlat:
//		gl_control->scene->FlatShading();
		break;
	case ESimpleCubeSmooth:
//		gl_control->scene->SmoothShading();
		break;
	case ESimpleCubeTriangles:
//		gl_control->scene->TriangleMode();
		break;
	case ESimpleCubeTriangleFans:
//		gl_control->scene->TriangleFanMode();
	break;
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
