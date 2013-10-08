#include "touch_tracker.h"
#include <algorithm>

namespace xPlatform
{

int eTouchTracker::Process(const void* touch, bool on)
{
	int id = -1;
	eTouches::iterator i = std::find(touches.begin(), touches.end(), touch);
	if(i == touches.end())
	{
		if(on)
		{
			i = std::find(touches.begin(), touches.end(), (void*)NULL);
			if(i == touches.end())
			{
				id = touches.size();
				touches.push_back(touch);
			}
			else
			{
				id = i - touches.begin();
				*i = touch;
			}
		}
	}
	else
	{
		id = i - touches.begin();
		if(!on)
		{
			*i = NULL;
		}
	}
	return id;
}

}
//namespace xPlatform
