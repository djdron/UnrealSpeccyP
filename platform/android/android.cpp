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

#include "../platform.h"

#ifdef _ANDROID

#include <jni.h>
#include "../io.h"
#include "../../options_common.h"
#include "../../tools/options.h"
#include "../touch_ui/tui_keyboard.h"
#include "../touch_ui/tui_joystick.h"

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

static struct eOptionZoom : public xOptions::eOptionInt
{
	eOptionZoom() { Set(0); }
	virtual const char* Name() const { return "zoom"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "fill", NULL };
		return values;
	}
	virtual int Order() const { return 1; }
} op_zoom;

static struct eOptionFiltering : public xOptions::eOptionBool
{
	eOptionFiltering() { Set(false); }
	virtual const char* Name() const { return "filtering"; }
	virtual int Order() const { return 2; }
} op_filtering;

static struct eOptionSkipFrames : public xOptions::eOptionInt
{
	eOptionSkipFrames() { Set(0); }
	virtual const char* Name() const { return "skip frames"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "2", "4", NULL };
		return values;
	}
	virtual int Order() const { return 3; }
} op_skip_frames;

static struct eOptionUseSensor : public xOptions::eOptionBool
{
	eOptionUseSensor() { Set(false); }
	virtual const char* Name() const { return "use sensor"; }
	virtual int Order() const { return 4; }
} op_use_sensor;

static struct eOptionUseKeyboard : public xOptions::eOptionBool
{
	eOptionUseKeyboard() { Set(true); }
	virtual const char* Name() const { return "use keyboard"; }
	virtual int Order() const { return 5; }
} op_use_keyboard;

static void InitResources(const byte* rom0, const byte* rom1, const byte* rom2, const byte* rom3)
{
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

void Java_app_usp_Emulator_InitResources(JNIEnv* env, jobject obj, jobject rom0_buf, jobject rom1_buf, jobject rom2_buf, jobject rom3_buf)
{
	const byte* rom0 = (const byte*)env->GetDirectBufferAddress(rom0_buf);
	const byte* rom1 = (const byte*)env->GetDirectBufferAddress(rom1_buf);
	const byte* rom2 = (const byte*)env->GetDirectBufferAddress(rom2_buf);
	const byte* rom3 = (const byte*)env->GetDirectBufferAddress(rom3_buf);
	xPlatform::InitResources(rom0, rom1, rom2, rom3);
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

void Java_app_usp_Emulator_Update(JNIEnv* env, jobject obj)
{
	xPlatform::Handler()->OnLoop();
}
void Java_app_usp_Emulator_UpdateVideo(JNIEnv* env, jobject obj, jobject byte_buffer)
{
	uint16_t* buf = (uint16_t*)env->GetDirectBufferAddress(byte_buffer);
	xPlatform::UpdateScreen(buf);
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

void Java_app_usp_Emulator_OnTouch(JNIEnv* env, jobject obj, jboolean keyboard, jfloat x, jfloat y, jboolean down, jint pointer_id)
{
	if(keyboard)
		xPlatform::OnTouchKey(x, y, down, pointer_id);
	else
		xPlatform::OnTouchJoy(x, y, down, pointer_id);
}

jboolean Java_app_usp_Emulator_Open(JNIEnv* env, jobject obj, jstring jfile)
{
    const char* file = env->GetStringUTFChars(jfile, NULL);
    bool ok = xPlatform::Handler()->OnOpenFile(file);
    env->ReleaseStringUTFChars(jfile, file);
    return ok;
}
jstring Java_app_usp_Emulator_GetLastFile(JNIEnv* env, jobject obj)
{
	return env->NewStringUTF(xPlatform::OpLastFile());
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

jint Java_app_usp_Emulator_TapeState(JNIEnv* env, jobject obj)
{
	using namespace xPlatform;
	switch(Handler()->OnAction(A_TAPE_QUERY))
	{
	case AR_TAPE_NOT_INSERTED:	return 0;
	case AR_TAPE_STOPPED:		return 1;
	case AR_TAPE_STARTED:		return 2;
	}
}
void Java_app_usp_Emulator_TapeToggle(JNIEnv* env, jobject obj)
{
	xPlatform::Handler()->OnAction(xPlatform::A_TAPE_TOGGLE);
}

jboolean Java_app_usp_Emulator_FileTypeSupported(JNIEnv* env, jobject obj, jstring jname)
{
    const char* name = env->GetStringUTFChars(jname, NULL);
	bool r = xPlatform::Handler()->FileTypeSupported(name);
    env->ReleaseStringUTFChars(jname, name);
    return r;
}

}
//extern "C"

#endif//_ANDROID
