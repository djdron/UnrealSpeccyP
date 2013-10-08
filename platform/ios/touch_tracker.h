#ifndef __TOUCH_TRACKER_H__
#define __TOUCH_TRACKER_H__

#include <vector>

#pragma once

namespace xPlatform
{

class eTouchTracker
{
public:
	int Process(const void* touch, bool on);
private:
	typedef std::vector<const void*> eTouches;
	eTouches touches;
};

}
//namespace xPlatform

#endif//__TOUCH_TRACKER_H__
