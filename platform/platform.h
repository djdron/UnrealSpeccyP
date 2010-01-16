#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../std_types.h"

#pragma once

#define USE_GL
#define USE_GLUT

namespace xPlatform
{

struct eHandler
{
	virtual void OnLoop() = 0;
	virtual byte* DrawData() = 0;
};

bool Init(int argc, char* argv[], eHandler* h);
void Loop();
void Done();

}
//namespace xPlatform

#endif//__PLATFORM_H__
