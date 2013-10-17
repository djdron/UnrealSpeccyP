#import "view.h"
#import "../touch_ui/tui_keyboard.h"
#import "../touch_ui/tui_joystick.h"
#import "../platform.h"
#import "../../tools/options.h"
#import "touch_tracker.h"
#undef self

using namespace xPlatform;

@implementation MyGLView
{
	eTouchTracker touch_tracker;
	xOptions::eOption<bool>* op_use_keyboard;
}

-(id)initWithFrame:(CGRect)aRect
{
	self = [super initWithFrame:aRect];
	if(self)
	{
		op_use_keyboard = xOptions::eOption<bool>::Find("use keyboard");
	}
	return self;
}

-(void) processTouches:(NSSet*)touches pressed:(bool)pressed down:(bool)down
{
	for(UITouch* touch in touches)
	{
		int id = touch_tracker.Process(touch, down);
		if(id != -1)
		{
			CGPoint loc = [touch locationInView:self];
			ePoint size(self.bounds.size.width, self.bounds.size.height);
			if(loc.y < size.y/2)
			{
				if(loc.x < size.x/2)
				{
					Handler()->OnKey('m', pressed);
				}
				else if(loc.x > size.x/2 && pressed)
				{
					SAFE_CALL(op_use_keyboard)->Change();
					_last_touch_time.SetCurrent();
				}
			}
			else
			{
				loc.y -= (size.y - _overlay_size.y);
				if(!op_use_keyboard || *op_use_keyboard)
					OnTouchKey(loc.x/size.x, loc.y/_overlay_size.y, down, id);
				else
					OnTouchJoy(loc.x/size.x, loc.y/_overlay_size.y, down, id);
				_last_touch_time.SetCurrent();
			}
		}
	}
}

- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:true down:true];
}
 
- (void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:false down:true];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:false down:false];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:false down:false];
}

@end
