/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "../std_types.h"

#pragma once

#if defined(_WINDOWS) || defined(_LINUX)
#define USE_OAL
#define USE_GL
//#define USE_GLUT
#define USE_WXWIDGETS
#define USE_CUSTOM_UI
#endif//_WINDOWS || _LINUX

//#define USE_LOG

namespace xPlatform
{

enum eKeyFlags
{
	KF_DOWN = 0x01,
	KF_SHIFT = 0x02,
	KF_CTRL = 0x04,
	KF_ALT = 0x08,

	KF_KEMPSTON = 0x10,
	KF_CURSOR = 0x20,
	KF_QAOP = 0x40,
	KF_SINCLAIR2 = 0x80
};
enum eMouseAction { MA_MOVE, MA_BUTTON, MA_WHEEL };
enum eAction { A_RESET, A_TAPE_TOGGLE, A_TAPE_FAST_TOGGLE, A_DRIVE_NEXT };
enum eActionResult
{
	AR_RESET_OK,
	AR_TAPE_STARTED, AR_TAPE_STOPPED, AR_TAPE_NOT_INSERTED,
	AR_TAPE_FAST_SET, AR_TAPE_FAST_RESET,
	AR_DRIVE_A, AR_DRIVE_B, AR_DRIVE_C, AR_DRIVE_D,
	AR_ERROR
};

struct eHandler
{
	eHandler();
	~eHandler();
	virtual void OnInit() = 0;
	virtual void OnDone() = 0;
	virtual void OnLoop() = 0;
	virtual const char* WindowCaption() = 0;
	virtual void OnKey(char key, dword flags) = 0;
	virtual void OnMouse(eMouseAction action, byte a, byte b) = 0;

	virtual bool OnOpenFile(const char* name) = 0;
	virtual eActionResult OnAction(eAction action) = 0;

	// data to draw
	virtual void* VideoData() = 0;
	// pause/resume function for sync video by audio
	virtual void VideoPaused(bool paused) = 0;
	// audio
	virtual int	AudioSources() = 0;
	virtual void* AudioData(int source) = 0;
	virtual dword AudioDataReady(int source) = 0;
	virtual void AudioDataUse(int source, dword size) = 0;

	virtual bool FullSpeed() const = 0;
};

eHandler* Handler();

}
//namespace xPlatform

#endif//__PLATFORM_H__
