#import <GLKit/GLKit.h>
#import "../../tools/point.h"
#import "../../tools/tick.h"

@interface MyGLView : GLKView
@property ePoint overlay_size;
@property eTick last_touch_time;
@end
