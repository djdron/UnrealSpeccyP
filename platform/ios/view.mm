/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#import "view.h"
#import "../touch_ui/tui_keyboard.h"
#import "../touch_ui/tui_joystick.h"
#import "../platform.h"
#import "../../tools/options.h"
#import "touch_tracker.h"

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
			loc.y -= size.y - _overlay_size.y;
			if(loc.y < 0)
			{
				if(loc.x < size.x/2)
				{
					Handler()->OnKey('m', pressed);
				}
				else if(loc.x > size.x/2 && !down)
				{
					SAFE_CALL(op_use_keyboard)->Change();
					_last_touch_time.SetCurrent();
				}
			}
			else
			{
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
