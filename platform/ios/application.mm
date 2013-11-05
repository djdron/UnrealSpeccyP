#import "application.h"
#import "view.h"
#import "../platform.h"
#import "../gles2/gles2.h"
#import "../gles2/gles2_sprite.h"
#import "../../options_common.h"
#import "../../tools/options.h"
#import "../io.h"
#import <sys/stat.h>
#undef self

namespace xPlatform
{
void InitSound();
void DoneSound();
void OnLoopSound();

static struct eOptionUseKeyboard : public xOptions::eOptionBool
{
	eOptionUseKeyboard() { Set(true); customizable = false; }
	virtual const char* Name() const { return "use keyboard"; }
	virtual int Order() const { return 6; }
} op_use_keyboard;

}
//namespace xPlatform

using namespace xPlatform;

@interface Overlay : NSObject
@property ePoint size;
-(void)Draw:(ePoint)size :(eTick)last_touch_time;
@end

@implementation Overlay
{
	GLuint keyboard_texture;
	eGLES2Sprite* keyboard_sprite;
	GLuint joystick_texture;
	eGLES2Sprite* joystick_sprite;
}

-(GLuint)LoadTexture:(NSString*)name :(ePoint*)size
{
	UIImage* image = [UIImage imageNamed:name];
	*size = ePoint(image.size.width*image.scale, image.size.height*image.scale);
	GLubyte* imageData = new GLubyte[size->x*size->y*4];
	memset(imageData, 0, size->x*size->y*4);
	CGContextRef imageContext = CGBitmapContextCreate(imageData, size->x, size->y, 8, size->x * 4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaNoneSkipLast);
	CGContextSetBlendMode(imageContext, kCGBlendModeCopy);
	CGContextDrawImage(imageContext, CGRectMake(0.0, 0.0, size->x, size->y), image.CGImage);
	CGContextRelease(imageContext);

	ePoint size_pot = NextPot(*size);
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
		keyboard_texture = [self LoadTexture:@"keyboard.png" :&_size];
		keyboard_sprite = new eGLES2Sprite(keyboard_texture, _size);
		ePoint joystick_size;
		joystick_texture = [self LoadTexture:@"joystick.png" :&joystick_size];
		joystick_sprite = new eGLES2Sprite(joystick_texture, joystick_size);
	}
	return self;
}

-(void)dealloc
{
	delete keyboard_sprite;
	glDeleteTextures(1, &keyboard_texture);
	delete joystick_sprite;
	glDeleteTextures(1, &joystick_texture);
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
		if(op_use_keyboard)
			keyboard_sprite->Draw(ePoint(0, 0), ePoint(size.x, _size.y), alpha);
		else
			joystick_sprite->Draw(ePoint(0, 0), ePoint(size.x, _size.y), alpha);
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
	_window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	MyGLView* view = [[MyGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
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
	
//	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
//	const char* path = paths.count ? [[paths objectAtIndex: 0] UTF8String] : NULL;
	const char* path = "/User/Documents/";
	xIo::SetProfilePath(path);
	mkdir(path, 0755);
	OpLastFile(path);

	using namespace xOptions;
	struct eOptionBX : public eOptionB
	{
		void Unuse() { customizable = false; storeable = false; }
	};
	eOptionBX* o = (eOptionBX*)eOptionB::Find("sound");
	SAFE_CALL(o)->Unuse();
	o = (eOptionBX*)eOptionB::Find("volume");
	SAFE_CALL(o)->Unuse();
	o = (eOptionBX*)eOptionB::Find("quit");
	SAFE_CALL(o)->Unuse();

	Handler()->OnInit();
	gles2 = eGLES2::Create();
	overlay = [[Overlay alloc] init];
	view.overlay_size = overlay.size/view.contentScaleFactor;

	InitSound();

	[_window setRootViewController:viewController];
	[_window makeKeyAndVisible];
	application.idleTimerDisabled = YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	xOptions::Store();
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
	size *= view.contentScaleFactor;
	if(size.x > size.y) // landscape
		gles2->Draw(ePoint(), size);
	else
		gles2->Draw(ePoint(0, overlay.size.y), ePoint(size.x, size.y - overlay.size.y));
	[overlay Draw:size :view.last_touch_time];
}

#pragma mark - GLKViewControllerDelegate

- (void)glkViewControllerUpdate:(MyGLController *)controller
{
	Handler()->OnLoop();
	OnLoopSound();
}

@end
