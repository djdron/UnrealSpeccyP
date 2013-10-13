#import "application.h"
#import "view.h"
#import "../platform.h"
#import "../gles2/gles2.h"
#import "../gles2/gles2_sprite.h"
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

@interface Overlay : NSObject
@property ePoint keyboard_size;
-(void)Draw:(ePoint)size :(eTick)last_touch_time;
@end

@implementation Overlay
{
	GLuint keyboard_texture;
	eGLES2Sprite* keyboard_sprite;
}

-(GLuint)LoadTexture:(NSString*)name :(ePoint*)size
{
	UIImage* image = [UIImage imageNamed:name];
	*size = ePoint(image.size.width, image.size.height);
	ePoint size_pot = NextPot(*size);
	GLubyte* imageData = new GLubyte[size->x*size->y*4];
	CGContextRef imageContext = CGBitmapContextCreate(imageData, size->x, size->y, 8, size->x * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaPremultipliedLast);
	CGContextSetBlendMode(imageContext, kCGBlendModeCopy);
	CGContextDrawImage(imageContext, CGRectMake(0.0, 0.0, size->x, size->y), image.CGImage);
	CGContextRelease(imageContext);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	GLubyte* zeroData = new GLubyte[size_pot.x*size_pot.y*4];
	memset(zeroData, 0, size_pot.x*size_pot.y*4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_pot.x, size_pot.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, zeroData);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size->x, size->y, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	delete[] zeroData;
	delete[] imageData;
	return texture;
}

- (id)init
{
	self = [super init];
	if(self)
	{
		keyboard_texture = [self LoadTexture:@"keyboard.png" :&_keyboard_size];
		keyboard_sprite = new eGLES2Sprite(keyboard_texture, _keyboard_size);
	}
	return self;
}

-(void)dealloc
{
	delete keyboard_sprite;
	glDeleteTextures(1, &keyboard_texture);
	[super dealloc];
}

-(void)Draw:(ePoint)size :(eTick)last_touch_time
{
	float alpha = 1.0f;
	if(size.x > size.y) // landscape
	{
		const float HIDE_TIME_MS = 4000;
		const float ALPHA_TIME_MS = 1000;
		float passed_time_ms = last_touch_time.Passed().Ms();
		if(passed_time_ms > HIDE_TIME_MS)
			alpha = 0.0f;
		else if(passed_time_ms > HIDE_TIME_MS - ALPHA_TIME_MS)
			alpha = (HIDE_TIME_MS - passed_time_ms)/ALPHA_TIME_MS*0.6f;
		else
			alpha *= 0.6f;
	}
	if(alpha > 0.0f)
	{
		keyboard_sprite->Draw(ePoint(0, 0), ePoint(size.x, _keyboard_size.y), alpha);
	}
}
@end

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
	Overlay* overlay;
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
	overlay = [[Overlay alloc] init];
	view.overlay_size = overlay.keyboard_size;

	InitSound();

	[_window setRootViewController:viewController];
	[_window makeKeyAndVisible];
	application.idleTimerDisabled = YES;
}

- (void)dealloc
{
	DoneSound();
	delete gles2;
	Handler()->OnDone();
	[super dealloc];
}

#pragma mark - GLKViewDelegate
 
- (void)glkView:(MyGLView*)view drawInRect:(CGRect)rect
{
	ePoint size(rect.size.width, rect.size.height);
	if(size.x > size.y) // landscape
		gles2->Draw(ePoint(), size);
	else
		gles2->Draw(ePoint(0, overlay.keyboard_size.y), ePoint(size.x, size.y - overlay.keyboard_size.y));
	[overlay Draw:size :view.last_touch_time];
}

#pragma mark - GLKViewControllerDelegate

- (void)glkViewControllerUpdate:(MyGLController *)controller
{
	Handler()->OnLoop();
	OnLoopSound();
}

@end
