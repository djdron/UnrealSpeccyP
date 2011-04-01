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
void ProcessKey(char key, bool down, bool shift, bool alt);
void InitSound();
void DoneSound();
int UpdateSound(byte* buf);

static void InitResources(const byte* font, const byte* rom0, const byte* rom1, const byte* rom2, const byte* rom3)
{
	memcpy(spxtrm4f, 	font, sizeof(spxtrm4f));
	memcpy(sos128,		rom0, sizeof(sos128));
	memcpy(sos48,		rom1, sizeof(sos48));
	memcpy(service,		rom2, sizeof(service));
	memcpy(dos513f,		rom3, sizeof(dos513f));
}

static void Init(const char* path)
{
	const char* res = "/";
	OpLastFile(res);
//	xIo::SetResourcePath(res);
	char buf[xIo::MAX_PATH_LEN];
	strcpy(buf, path);
	strcat(buf, "/");
	xIo::SetProfilePath(buf);
	Handler()->OnInit();
	InitSound();
}
static void Done()
{
	DoneSound();
	Handler()->OnDone();
}

}
//namespace xPlatform

template<class T> static int GetOption(const char* name)
{
	xOptions::eOption<T>* o = xOptions::eOption<T>::Find(name);
	return o ? *o : T(0);
}
template<class T> static int SetOption(const char* name, const T& value)
{
	xOptions::eOption<T>* o = xOptions::eOption<T>::Find(name);
	if(o)
	{
		o->Set(value);
		o->Apply();
	}
}

extern "C"
{

void Java_app_usp_Emulator_InitResources(JNIEnv* env, jobject obj, jobject font_buf, jobject rom0_buf, jobject rom1_buf, jobject rom2_buf, jobject rom3_buf)
{
	const byte* font = (const byte*)env->GetDirectBufferAddress(font_buf);
	const byte* rom0 = (const byte*)env->GetDirectBufferAddress(rom0_buf);
	const byte* rom1 = (const byte*)env->GetDirectBufferAddress(rom1_buf);
	const byte* rom2 = (const byte*)env->GetDirectBufferAddress(rom2_buf);
	const byte* rom3 = (const byte*)env->GetDirectBufferAddress(rom3_buf);
	xPlatform::InitResources(font, rom0, rom1, rom2, rom3);
}

void Java_app_usp_Emulator_Init(JNIEnv* env, jobject obj, jstring jpath)
{
    const char* path = env->GetStringUTFChars(jpath, NULL);
	xPlatform::Init(path);
    env->ReleaseStringUTFChars(jpath, path);
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

void Java_app_usp_Emulator_OnKey(JNIEnv* env, jobject obj, jchar key, jboolean down, jboolean shift, jboolean alt)
{
	xPlatform::ProcessKey(key, down, shift, alt);
}

void Java_app_usp_Emulator_Open(JNIEnv* env, jobject obj, jstring jfile)
{
    const char* file = env->GetStringUTFChars(jfile, NULL);
    xPlatform::Handler()->OnOpenFile(file);
    env->ReleaseStringUTFChars(jfile, file);
}
jstring Java_app_usp_Emulator_GetLastFolder(JNIEnv* env, jobject obj)
{
	return env->NewStringUTF(xPlatform::OpLastFolder());
}

void Java_app_usp_Emulator_LoadState(JNIEnv* env, jobject obj)
{
	using namespace xOptions;
	eOptionB* o = eOptionB::Find("load state");
	SAFE_CALL(o)->Change();
}

void Java_app_usp_Emulator_SaveState(JNIEnv* env, jobject obj)
{
	using namespace xOptions;
	eOptionB* o = eOptionB::Find("save state");
	SAFE_CALL(o)->Change();
}

void Java_app_usp_Emulator_StoreOptions(JNIEnv* env, jobject obj)
{
	xOptions::Store();
}

void Java_app_usp_Emulator_Reset(JNIEnv* env, jobject obj)
{
	xPlatform::Handler()->OnAction(xPlatform::A_RESET);
}

jint Java_app_usp_Emulator_GetOptionInt(JNIEnv* env, jobject obj, jstring jname)
{
    const char* name = env->GetStringUTFChars(jname, NULL);
	int r = GetOption<int>(name);
    env->ReleaseStringUTFChars(jname, name);
    return r;
}
void Java_app_usp_Emulator_SetOptionInt(JNIEnv* env, jobject obj, jstring jname, jint value)
{
    const char* name = env->GetStringUTFChars(jname, NULL);
	SetOption<int>(name, value);
    env->ReleaseStringUTFChars(jname, name);
}

jboolean Java_app_usp_Emulator_GetOptionBool(JNIEnv* env, jobject obj, jstring jname)
{
    const char* name = env->GetStringUTFChars(jname, NULL);
	int r = GetOption<bool>(name);
    env->ReleaseStringUTFChars(jname, name);
    return r;
}
void Java_app_usp_Emulator_SetOptionBool(JNIEnv* env, jobject obj, jstring jname, jboolean value)
{
    const char* name = env->GetStringUTFChars(jname, NULL);
	SetOption<bool>(name, value);
    env->ReleaseStringUTFChars(jname, name);
}

}
//extern "C"

#endif//_ANDROID
