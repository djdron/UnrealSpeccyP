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
#include "../../../tools/options.h"

byte spxtrm4f[2048];
byte sos128[16384];
byte sos48[16384];
byte service[16384];
byte dos513f[16384];

namespace xPlatform
{
void UpdateScreen(word* scr);
void ProcessKey(char key, bool down);
void InitSound();
void DoneSound();
int UpdateSound(byte* buf);

void Init(const byte* font, const byte* rom0, const byte* rom1, const byte* rom2, const byte* rom3)
{
	memcpy(spxtrm4f, 	font, sizeof(spxtrm4f));
	memcpy(sos128,		rom0, sizeof(sos128));
	memcpy(sos48,		rom1, sizeof(sos48));
	memcpy(service,		rom2, sizeof(service));
	memcpy(dos513f,		rom3, sizeof(dos513f));
	const char* res = "/sdcard/";
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

void Java_app_usp_Emulator_Init(JNIEnv* env, jobject obj, jobject font_buf, jobject rom0_buf, jobject rom1_buf, jobject rom2_buf, jobject rom3_buf)
{
	const byte* font = (const byte*)env->GetDirectBufferAddress(font_buf);
	const byte* rom0 = (const byte*)env->GetDirectBufferAddress(rom0_buf);
	const byte* rom1 = (const byte*)env->GetDirectBufferAddress(rom1_buf);
	const byte* rom2 = (const byte*)env->GetDirectBufferAddress(rom2_buf);
	const byte* rom3 = (const byte*)env->GetDirectBufferAddress(rom3_buf);
	xPlatform::Init(font, rom0, rom1, rom2, rom3);
}
void Java_app_usp_Emulator_Done(JNIEnv* env, jobject obj)
{
	xPlatform::Done();
}

jboolean Java_app_usp_Emulator_UpdateVideo(JNIEnv* env, jobject obj, jobject byte_buffer)
{
	xPlatform::Handler()->OnLoop();
	uint16_t* buf = (uint16_t*)env->GetDirectBufferAddress(byte_buffer);
	xPlatform::UpdateScreen(buf);
	bool quit = xPlatform::OpQuit();
	if(quit)
		xPlatform::OpQuit(false);
	return !quit;
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

void Java_app_usp_Emulator_StoreOptions(JNIEnv* env, jobject obj)
{
	xOptions::Store();
}

}
//extern "C"

#endif//_ANDROID
