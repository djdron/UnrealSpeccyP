#include "../../std.h"

#ifdef _SYMBIAN

#include "../platform.h"

#include <eikstart.h>
#include <eikapp.h>
#include <aknapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknappui.h>

#include <unreal_speccy_portable.rsg>
#include "../../symbian/unreal_speccy_portable.hrh"

#include <avkon.hrh>

#include <coecntrl.h>
#include <GLES/egl.h>
#include <akndef.h>
#include <akndoc.h>

#include <e32base.h>
#include <GLES/gl.h>

typedef enum { ETriangles, ETriangleFans} TRenderingMode;

#define FRUSTUM_LEFT   -1.f     //left vertical clipping plane
#define FRUSTUM_RIGHT   1.f     //right vertical clipping plane
#define FRUSTUM_BOTTOM -1.f     //bottom horizontal clipping plane
#define FRUSTUM_TOP     1.f     //top horizontal clipping plane
#define FRUSTUM_NEAR    3.f     //near depth clipping plane
#define FRUSTUM_FAR  1000.f     //far depth clipping plane

class CSimpleCube : public CBase
{
public:
	static CSimpleCube* NewL( TUint aWidth, TUint aHeight)
	{
		CSimpleCube* self = new (ELeave) CSimpleCube( aWidth, aHeight );
		CleanupStack::PushL( self );
		self->ConstructL();
		CleanupStack::Pop();
		return self;
	}
	virtual ~CSimpleCube()
	{}

public:
	void AppInit( void );
	void AppExit( void );
	void DrawBox( GLfloat aSizeX, GLfloat aSizeY, GLfloat aSizeZ );
	void AppCycle( TInt aFrame );
	void FlatShading( void );
	void SmoothShading( void );
	void TriangleMode( void );
	void TriangleFanMode( void );
	void SetScreenSize( TUint aWidth, TUint aHeight );
protected:
	CSimpleCube(TUint aWidth, TUint aHeight) : iScreenWidth(aWidth), iScreenHeight(aHeight)
	{}
private:
	TUint iScreenWidth;
	TUint iScreenHeight;
	TRenderingMode iDrawingMode;
};
/** Vertice coordinates for the cube. */
static const GLbyte vertices[8 * 3] =
{
	-1, 1, 1,
	1, 1, 1,
	1, -1, 1,
	-1, -1, 1,

	-1, 1, -1,
	1, 1, -1,
	1, -1, -1,
	-1, -1, -1
};

/** Colors for vertices (Red, Green, Blue, Alpha). */
static const GLubyte colors[8 * 4] =
{
	0 ,255, 0,255,
	0 , 0,255,255,
	0 ,255, 0,255,
	255, 0, 0,255,

	0 , 0,255,255,
	255, 0, 0,255,
	0 , 0,255,255,
	0 ,255, 0,255
};

/**
 * Indices for drawing the triangles.
 * The color of the triangle is determined by
 * the color of the last vertex of the triangle.
 */
static const GLubyte triangles[12 * 3] =
{
	/* front */
	1,0,3,
	1,3,2,

	/* right */
	2,6,5,
	2,5,1,

	/* back */
	7,4,5,
	7,5,6,

	/* left */
	0,4,7,
	0,7,3,

	/* top */
	5,4,0,
	5,0,1,

	/* bottom */
	3,7,6,
	3,6,2
};

/** First set of indices for drawing the triangle fans. */
static const GLubyte fanOne[6 * 3] =
{
	1,0,3,
	1,3,2,
	1,2,6,
	1,6,5,
	1,5,4,
	1,4,0
};

/** Second set of indices for drawing the triangle fans. */
static const GLubyte fanTwo[6 * 3] =
{
	7,4,5,
	7,5,6,
	7,6,2,
	7,2,3,
	7,3,0,
	7,0,4
};

void CSimpleCube::AppInit( void )
{
	// Set the screen background color.
	glClearColor( 0.f, 0.f, 0.f, 1.f );

	// Enable back face culling.
	glEnable( GL_CULL_FACE );

	// Initialize viewport and projection.
	glViewport( 0, 0, iScreenWidth, iScreenHeight );
	glMatrixMode( GL_PROJECTION );

	// Calculate the view frustrum
	GLfloat aspectRatio = (GLfloat)(iScreenWidth) / (GLfloat)(iScreenHeight);
	glFrustumf( FRUSTUM_LEFT * aspectRatio, FRUSTUM_RIGHT * aspectRatio,
			FRUSTUM_BOTTOM, FRUSTUM_TOP,
			FRUSTUM_NEAR, FRUSTUM_FAR );

	glMatrixMode( GL_MODELVIEW );

	// Enable vertex arrays.
	glEnableClientState( GL_VERTEX_ARRAY );

	// Set array pointers.
	glVertexPointer( 3, GL_BYTE, 0, vertices );

	// Enable color arrays.
	glEnableClientState( GL_COLOR_ARRAY );

	// Set color pointers.
	glColorPointer( 4, GL_UNSIGNED_BYTE, 0, colors );

	// Set the initial shading mode
	glShadeModel( GL_FLAT );

	// Do not use perspective correction
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );

	// Set the initial drawing mode
	iDrawingMode = ETriangles;
}

// -----------------------------------------------------------------------------
// CSimpleCube::AppExit
//
// Release any allocations made in AppInit.
// -----------------------------------------------------------------------------
//

void CSimpleCube::AppExit( void )
{
}

// -----------------------------------------------------------------------------
// CSimpleCube::DrawBox
//
// Draws a box with triangles or triangle fans depending on the current rendering mode.
// Scales the box to the given size using glScalef.
// -----------------------------------------------------------------------------
//

void CSimpleCube::DrawBox( GLfloat aSizeX, GLfloat aSizeY, GLfloat aSizeZ )
{
	glScalef( aSizeX, aSizeY, aSizeZ );

	if ( iDrawingMode == ETriangles )
	{
		glDrawElements( GL_TRIANGLES, 12 * 3, GL_UNSIGNED_BYTE, triangles );
	}
	else if ( iDrawingMode == ETriangleFans )
	{
		glDrawElements( GL_TRIANGLE_FAN, 6 * 3, GL_UNSIGNED_BYTE, fanOne );
		glDrawElements( GL_TRIANGLE_FAN, 6 * 3, GL_UNSIGNED_BYTE, fanTwo );
	}
}

// -----------------------------------------------------------------------------
// CSimpleCube::AppCycle
//
// Draws and animates the objects.
// The frame number determines the amount of rotation.
// -----------------------------------------------------------------------------
//

void CSimpleCube::AppCycle( TInt aFrame )
{
	const GLint cameraDistance = 100;

	glClear( GL_COLOR_BUFFER_BIT );

	/* Animate and draw box */
	glLoadIdentity();
	glTranslatex( 0 , 0 , -cameraDistance << 16 );
	glRotatex( aFrame << 16, 1 << 16, 0 , 0 );
	glRotatex( aFrame << 15, 0 , 1 << 16, 0 );
	glRotatex( aFrame << 14, 0 , 0 , 1 << 16 );
	DrawBox( 15.f, 15.f, 15.f );
}

//----------------------------------------------------------
// The following methods are called by the CSimpleCubeAppUi
// class when handling the incoming menu events.
//----------------------------------------------------------

// -----------------------------------------------------------------------------
// CSimpleCube::FlatShading
//
// Sets the GL shading model to flat.
// -----------------------------------------------------------------------------
//

void CSimpleCube::FlatShading( void )
{
	glShadeModel( GL_FLAT );
}

// -----------------------------------------------------------------------------
// CSimpleCube::SmoothShading
//
// Sets the GL shading model to smooth.
// -----------------------------------------------------------------------------
//

void CSimpleCube::SmoothShading( void )
{
	glShadeModel( GL_SMOOTH );
}

// -----------------------------------------------------------------------------
// CSimpleCube::TriangleMode
//
// Sets the rendering mode to triangles.
// -----------------------------------------------------------------------------
//

void CSimpleCube::TriangleMode( void )
{
	iDrawingMode = ETriangles;
}

// -----------------------------------------------------------------------------
// CSimpleCube::TriangleFanMode
//
// Sets the rendering mode to triangle fans.
// -----------------------------------------------------------------------------
//

void CSimpleCube::TriangleFanMode( void )
{
	iDrawingMode = ETriangleFans;
}

// -----------------------------------------------------------------------------
// CSimpleCube::SetScreenSize
// Reacts to the dynamic screen size change during execution of this program.
// -----------------------------------------------------------------------------
//
void CSimpleCube::SetScreenSize( TUint aWidth, TUint aHeight )
{
	iScreenWidth = aWidth;
	iScreenHeight = aHeight;

	// Reinitialize viewport and projection.
	glViewport( 0, 0, iScreenWidth, iScreenHeight );

	// Recalculate the view frustrum
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	GLfloat aspectRatio = (GLfloat)(iScreenWidth) / (GLfloat)(iScreenHeight);
	glFrustumf( FRUSTUM_LEFT * aspectRatio, FRUSTUM_RIGHT * aspectRatio,
			FRUSTUM_BOTTOM, FRUSTUM_TOP,
			FRUSTUM_NEAR, FRUSTUM_FAR );
	glMatrixMode( GL_MODELVIEW );
}

class CSimpleCubeContainer : public CCoeControl, MCoeControlObserver
{
public:
	void ConstructL(const TRect& aRect);
	virtual ~CSimpleCubeContainer();

public:
	static TInt DrawCallBack(TAny* aInstance);

private:
	void SizeChanged();
	void HandleResourceChange(TInt aType);
	TInt CountComponentControls() const
	{	return 0;}
	CCoeControl* ComponentControl(TInt aIndex) const
	{	return NULL;}
	void Draw(const TRect& aRect) const;
	void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

private:
	EGLDisplay iEglDisplay;
	EGLSurface iEglSurface;
	EGLContext iEglContext;
	CPeriodic* iPeriodic;
	TBool iOpenGlInitialized;
	TInt iFrame;
public:
	CSimpleCube* iSimpleCube;
};

void CSimpleCubeContainer::ConstructL(const TRect& /*aRect*/)
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

	switch ( DMode )
	{
		case(EColor4K):
		BufferSize = 12;
		break;
		case(EColor64K):
		BufferSize = 16;
		break;
		case(EColor16M):
		BufferSize = 24;
		break;
		case(EColor16MU):
		BufferSize = 32;
		break;
		default:
		_LIT(KDModeError, "unsupported displaymode");
		User::Panic( KDModeError, 0 );
		break;
	}

	const EGLint attrib_list[] =
	{	EGL_BUFFER_SIZE,BufferSize,
		EGL_NONE};

	if ( eglChooseConfig( iEglDisplay, attrib_list, configList, configSize,
					&numOfConfigs ) == EGL_FALSE )
	{
		_LIT(KChooseConfigFailed, "eglChooseConfig failed");
		User::Panic( KChooseConfigFailed, 0 );
	}

	Config = configList[0];
	User::Free( configList );

	iEglSurface = eglCreateWindowSurface( iEglDisplay, Config, &Window(), NULL );
	if ( iEglSurface == NULL )
	{
		_LIT(KCreateWindowSurfaceFailed, "eglCreateWindowSurface failed");
		User::Panic( KCreateWindowSurfaceFailed, 0 );
	}

	iEglContext = eglCreateContext( iEglDisplay, Config, EGL_NO_CONTEXT, NULL );
	if ( iEglContext == NULL )
	{
		_LIT(KCreateContextFailed, "eglCreateContext failed");
		User::Panic( KCreateContextFailed, 0 );
	}

	if ( eglMakeCurrent( iEglDisplay, iEglSurface, iEglSurface, iEglContext )
			== EGL_FALSE )
	{
		_LIT(KMakeCurrentFailed, "eglMakeCurrent failed");
		User::Panic( KMakeCurrentFailed, 0 );
	}

	TSize size;
	size = this->Size();

	iSimpleCube = CSimpleCube::NewL( size.iWidth, size.iHeight );

	iOpenGlInitialized = ETrue;

	iPeriodic = CPeriodic::NewL( CActive::EPriorityIdle );
	iPeriodic->Start( 100, 100,
			TCallBack( CSimpleCubeContainer::DrawCallBack, this ) );

}

CSimpleCubeContainer::~CSimpleCubeContainer()
{
	delete iPeriodic;

	if ( iSimpleCube )
	{
		iSimpleCube->AppExit();
		delete iSimpleCube;
	}

	eglMakeCurrent( iEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroySurface( iEglDisplay, iEglSurface );
	eglDestroyContext( iEglDisplay, iEglContext );
	eglTerminate( iEglDisplay );
}

void CSimpleCubeContainer::SizeChanged()
{
	if( iOpenGlInitialized && iSimpleCube )
	{
		TSize size;
		size = this->Size();

		iSimpleCube->SetScreenSize( size.iWidth, size.iHeight );
	}
}

void CSimpleCubeContainer::HandleResourceChange(TInt aType)
{
	switch( aType )
	{
		case KEikDynamicLayoutVariantSwitch:
		SetExtentToWholeScreen();
		break;
	}
}

void CSimpleCubeContainer::Draw(const TRect& /*aRect*/) const
{
	CWindowGc& gc = SystemGc();
	gc.Clear( Rect() );
}

TInt CSimpleCubeContainer::DrawCallBack( TAny* aInstance )
{
	CSimpleCubeContainer* instance = (CSimpleCubeContainer*) aInstance;
	instance->iFrame++;

	instance->iSimpleCube->AppCycle( instance->iFrame );

	eglSwapBuffers( instance->iEglDisplay, instance->iEglSurface );

	if ( !(instance->iFrame%100) )
	{
		User::ResetInactivityTime();
	}

	if ( !(instance->iFrame%50) )
	{
		User::After(0);
	}

	return 0;
}

class CEikAppUi;
class CSimpleCubeDocument : public CAknDocument
{
public:
	static CSimpleCubeDocument* NewL(CEikApplication& aApp)
	{
		CSimpleCubeDocument* self = new (ELeave) CSimpleCubeDocument( aApp );
		CleanupStack::PushL( self );
		self->ConstructL();
		CleanupStack::Pop();
		return self;
	}
	virtual ~CSimpleCubeDocument()
	{}

private:
	CSimpleCubeDocument(CEikApplication& aApp) : CAknDocument(aApp)
	{}
	void ConstructL()
	{}

private:
	CEikAppUi* CreateAppUiL()
	{
		return new (ELeave) CSimpleCubeAppUi;
	}
};

class CSimpleCubeAppUi : public CAknAppUi
{
public:
	void ConstructL()
	{
		BaseConstructL();
		iAppContainer = new (ELeave) CSimpleCubeContainer;
		iAppContainer->SetMopParent(this);
		iAppContainer->ConstructL( ClientRect() );
		AddToStackL( iAppContainer );
	}
	virtual ~CSimpleCubeAppUi()
	{
		if(iAppContainer)
		{
			RemoveFromStack(iAppContainer);
			delete iAppContainer;
		}
	}

private:
	void HandleCommandL(TInt aCommand);
	virtual TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
		return EKeyWasNotConsumed;
	}

private:
	CSimpleCubeContainer* iAppContainer;
};

void CSimpleCubeAppUi::HandleCommandL(TInt aCommand)
{
	switch(aCommand)
	{
		case EAknSoftkeyBack:
		case EEikCmdExit:
		Exit();
		break;
		case ESimpleCubeFlat:
		iAppContainer->iSimpleCube->FlatShading();
		break;
		case ESimpleCubeSmooth:
		iAppContainer->iSimpleCube->SmoothShading();
		break;
		case ESimpleCubeTriangles:
		iAppContainer->iSimpleCube->TriangleMode();
		break;
		case ESimpleCubeTriangleFans:
		iAppContainer->iSimpleCube->TriangleFanMode();
		break;
		default:
		break;
	}
}

const TUid KUidSimpleCube =
{	0xA000821A};

class CSimpleCubeApp : public CAknApplication
{
private:
	CApaDocument* CreateDocumentL()
	{	return CSimpleCubeDocument::NewL(*this);}
	TUid AppDllUid() const
	{	return KUidSimpleCube;}
};

LOCAL_C CApaApplication* NewApplication()
{
	return new CSimpleCubeApp;
}
GLDEF_C TInt E32Main()
{
	return EikStart::RunApplication(NewApplication);
}

#endif//_SYMBIAN
