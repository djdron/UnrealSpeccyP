#import "application.h"
#import "view.h"
#import "../platform.h"
#import "../gles2/gles2.h"
#import "../../options_common.h"
#import "../io.h"
#undef self

namespace xPlatform
{
void InitSound();
void DoneSound();
void OnLoopSound();
}
//namespace xPlatform

using namespace xPlatform;

@interface MyGLController : GLKViewController
@end

@implementation MyGLController
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   return YES;
}
@end

@implementation USPApplication
{
	UIWindow* _window;
	eGLES2* gles2;
}

@synthesize window = _window;

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	_window		= [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	MyGLView* view	= [[MyGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[view setMultipleTouchEnabled:YES];
	EAGLContext* context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	[EAGLContext setCurrentContext:context];
	view.context = context;
	view.delegate = self;

	MyGLController* viewController = [[MyGLController alloc] init];
	viewController.view = view;
	viewController.delegate = self;
	viewController.preferredFramesPerSecond = 60;
	
	NSString* bundle_res_path = [[[NSBundle mainBundle] resourcePath] stringByAppendingString:@"/"];
	xIo::SetResourcePath([bundle_res_path UTF8String]);
	OpLastFile("/");
	Handler()->OnInit();
//	Handler()->OnOpenFile(xIo::ResourcePath("rick1.rzx"));
	gles2 = eGLES2::Create();
	InitSound();

	[_window setRootViewController:viewController];
	[_window makeKeyAndVisible];
	application.idleTimerDisabled = YES;
}

- (void) dealloc
{
	DoneSound();
	delete gles2;
	Handler()->OnDone();
	[super dealloc];
}

#pragma mark - GLKViewDelegate
 
- (void)glkView:(MyGLView*)view drawInRect:(CGRect)rect
{
	gles2->Draw(rect.size.width, rect.size.height);
}

#pragma mark - GLKViewControllerDelegate

- (void)glkViewControllerUpdate:(MyGLController *)controller
{
	Handler()->OnLoop();
	OnLoopSound();
}

@end
