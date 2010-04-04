#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../std_types.h"

#pragma once

#if defined(_WINDOWS) || defined(_LINUX)
#define USE_OAL
#define USE_GL
//#define USE_GLUT
#define USE_WXWIDGETS
#endif//_WINDOWS || _LINUX

//#define USE_LOG

namespace xPlatform
{

enum eKeyFlags { KF_DOWN = 0x01, KF_SHIFT = 0x08, KF_CTRL = 0x02, KF_ALT = 0x04 };
enum eMouseAction { MA_MOVE, MA_BUTTON, MA_WHEEL };
enum eAction { A_RESET, A_TAPE_TOGGLE, A_DRIVE_NEXT };
enum eActionResult
{
	AR_RESET_OK,
	AR_TAPE_STARTED, AR_TAPE_STOPPED, AR_TAPE_NOT_INSERTED,
	AR_DRIVE_A, AR_DRIVE_B, AR_DRIVE_C, AR_DRIVE_D,
	AR_ERROR
};

struct eHandler
{
	eHandler();
	virtual ~eHandler();
	virtual void OnInit() {}
	virtual void OnDone() {}
	virtual void OnLoop() {}
	virtual const char* WindowCaption() { return NULL; }
	virtual void OnKey(char key, dword flags) {}
	virtual void OnMouse(eMouseAction action, byte a, byte b) {}

	virtual bool OnOpenFile(const char* name) { return false; }
	virtual eActionResult OnAction(eAction action) { return AR_ERROR; }

	// data to draw
	virtual void* VideoData() { return NULL; }
	// pause/resume function for sync video by audio
	virtual void VideoPaused(bool paused) {}
	// audio
	virtual int	AudioSources() { return 0; }
	virtual void* AudioData(int source) { return NULL; }
	virtual dword AudioDataReady(int source) { return 0; }
	virtual void AudioDataUse(int source, dword size) {}
};

eHandler* Handler();

}
//namespace xPlatform

#endif//__PLATFORM_H__
