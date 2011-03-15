/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../../platform.h"

#ifdef _ANDROID

#include <jni.h>
#include "../../io.h"
#include "../../../options_common.h"

namespace xPlatform
{
void UpdateScreen(word* scr);
void ProcessKey(char key, bool down);
void InitSound();
void DoneSound();
int UpdateSound(byte* buf);
void Init()
{
	const char* res = "/sdcard/usp/";
	OpLastFile(res);
	xIo::SetResourcePath(res);
	xIo::SetProfilePath(res);
	Handler()->OnInit();
	InitSound();
}
void Done()
{
	DoneSound();
	Handler()->OnDone();
}

}
//namespace xPlatform

extern "C"
{

void Java_app_usp_Emulator_Init(JNIEnv* env, jobject obj)
{
	xPlatform::Init();
}
void Java_app_usp_Emulator_Done(JNIEnv* env, jobject obj)
{
	xPlatform::Done();
}

void Java_app_usp_Emulator_UpdateVideo(JNIEnv* env, jobject obj, jobject byte_buffer)
{
	xPlatform::Handler()->OnLoop();
	uint16_t* buf = (uint16_t*)env->GetDirectBufferAddress(byte_buffer);
	xPlatform::UpdateScreen(buf);
}
jint Java_app_usp_Emulator_UpdateAudio(JNIEnv* env, jobject obj, jobject byte_buffer)
{
	byte* buf = (byte*)env->GetDirectBufferAddress(byte_buffer);
	return xPlatform::UpdateSound(buf);
}

void Java_app_usp_Emulator_OnKey(JNIEnv* env, jobject obj, jchar key, jboolean down)
{
	xPlatform::ProcessKey(key, down);
}

}
//extern "C"

#endif//_ANDROID
