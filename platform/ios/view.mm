#import "view.h"
#import "../touch_ui/tui_keyboard.h"
#import "../platform.h"
#import "touch_tracker.h"
#undef self

using namespace xPlatform;

@implementation MyGLView

eTouchTracker touch_tracker;

-(void) processTouches:(NSSet*)touches pressed:(bool)on
{
	for(UITouch* touch in touches)
	{
		int id = touch_tracker.Process(touch, on);
		if(id != -1)
		{
			CGPoint loc = [touch locationInView:self];
			int w = self.bounds.size.width, h = self.bounds.size.height;
			if(loc.y < h/2)
			{
				if(loc.x < w/2)
				{
					Handler()->OnKey('m', on ? KF_DOWN : 0);
				}
				else if(loc.x > w/2)
				{
					//toggle keyboard/joystick
				}
			}
			else
			{
				loc.y -= h/2;
				OnTouchKey(loc.x/w, loc.y*2/h, on, id);
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
