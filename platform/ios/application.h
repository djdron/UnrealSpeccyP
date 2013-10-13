#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface USPApplication : UIResponder <UIApplicationDelegate, GLKViewDelegate, GLKViewControllerDelegate>
@property (nonatomic, retain) UIWindow* window;
@end
