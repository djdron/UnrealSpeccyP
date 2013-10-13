#import "view.h"
#import "../touch_ui/tui_keyboard.h"
#import "../platform.h"
#import "touch_tracker.h"
#undef self

using namespace xPlatform;

@implementation MyGLView
{
	eTouchTracker touch_tracker;
}

-(void) processTouches:(NSSet*)touches pressed:(bool)on
{
	for(UITouch* touch in touches)
	{
		int id = touch_tracker.Process(touch, on);
		if(id != -1)
		{
			CGPoint loc = [touch locationInView:self];
			ePoint size(self.bounds.size.width, self.bounds.size.height);
			if(loc.y < size.y/2)
			{
				if(loc.x < size.x/2)
				{
					Handler()->OnKey('m', on ? KF_DOWN : 0);
				}
				else if(loc.x > size.x/2)
				{
					//toggle keyboard/joystick
				}
			}
			else
			{
				loc.y -= (size.y - _overlay_size.y);
				OnTouchKey(loc.x/size.x, loc.y/_overlay_size.y, on, id);
				_last_touch_time.SetCurrent();
			}
		}
	}
}

- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:true];
}
 
- (void) touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:true];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:false];
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self processTouches:touches pressed:false];
}

@end
