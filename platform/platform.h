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

#include "../std.h"

#pragma once

#if defined(_WINDOWS) || defined(_LINUX) || defined(_MAC)

#ifndef USE_SDL
#define USE_OAL
#define USE_GL
//#define USE_GLUT
#define USE_WXWIDGETS
#endif//USE_SDL

#define USE_PNG
#define USE_CONFIG
#define USE_ZIP

#endif//_WINDOWS || _LINUX || _MAC

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
	KF_SINCLAIR2 = 0x80,

	KF_UI_SENDER = 0x100
};
enum eMouseAction { MA_MOVE, MA_BUTTON, MA_WHEEL };
enum eAction
{
	A_RESET, A_TAPE_TOGGLE, A_TAPE_QUERY
};
enum eActionResult
{
	AR_OK,
	AR_TAPE_STARTED, AR_TAPE_STOPPED, AR_TAPE_NOT_INSERTED,
	AR_ERROR = -1
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
	virtual bool OnSaveFile(const char* name) = 0;
	virtual bool FileTypeSupported(const char* name) = 0;
	virtual eActionResult OnAction(eAction action) = 0;

	// data to draw
	virtual void* VideoData() = 0;
	virtual void* VideoDataUI() = 0;
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
