#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../std_types.h"

#pragma once

#define USE_GL
#define USE_GLUT

namespace xPlatform
{

enum eKeyFlags { KF_DOWN = 0x01, KF_SHIFT = 0x08, KF_CTRL = 0x02, KF_ALT = 0x04 };

struct eHandler
{
	virtual void OnLoop() = 0;
	virtual byte* DrawData() = 0;
	virtual const char* WindowCaption() = 0;
	virtual void OnKey(char key, dword flags) = 0;
};

bool Init(int argc, char* argv[], eHandler* h);
void Loop();
void Done();

}
//namespace xPlatform

#endif//__PLATFORM_H__
