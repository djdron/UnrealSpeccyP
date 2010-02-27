#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../std_types.h"

#pragma once

#define USE_OAL

#define USE_GL
//#define USE_GLUT
#define USE_WXWIDGETS

namespace xPlatform
{

enum eKeyFlags { KF_DOWN = 0x01, KF_SHIFT = 0x08, KF_CTRL = 0x02, KF_ALT = 0x04 };
enum eMouseAction { MA_MOVE, MA_BUTTON, MA_WHEEL };

struct eHandler
{
	eHandler();
	~eHandler();
	virtual void OnLoop() = 0;
	virtual const char* WindowCaption() = 0;
	virtual void OnKey(char key, dword flags) = 0;
	virtual void OnMouse(eMouseAction action, byte a, byte b) = 0;

	virtual void OnOpenFile(const char* name) = 0;
	virtual void OnReset() = 0;
	virtual void OnTape(bool start) = 0;

	// data to draw
	virtual void* VideoData() = 0;
	// pause/resume function for sync video by audio
	virtual void VideoPaused(bool paused) = 0;
	// audio
	virtual int	AudioSources() = 0;
	virtual void* AudioData(int source) = 0;
	virtual dword AudioDataReady(int source) = 0;
	virtual void AudioDataUse(int source, dword size) = 0;
};

eHandler* Handler();

}
//namespace xPlatform

#endif//__PLATFORM_H__
