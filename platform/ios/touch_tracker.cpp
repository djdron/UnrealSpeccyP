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
