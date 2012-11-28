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
#include "../../tools/profiler.h"
#include "../touch_ui/tui_keyboard.h"
#include "../touch_ui/tui_joystick.h"

PROFILER_DECLARE(u_vid);
PROFILER_DECLARE(u_aud);
PROFILER_DECLARE(pro0);
PROFILER_DECLARE(pro1);
PROFILER_DECLARE(pro2);
PROFILER_DECLARE(pro3);

byte sos128_0[16384];
byte sos128_1[16384];
byte sos48[16384];
byte service[16384];
byte dos513f[16384];
byte spxtrm4f[2048];

namespace xPlatform
{
void UpdateScreen(word* scr);
void ProcessKey(char key, bool down, bool shift, bool alt);
void InitSound();
void DoneSound();
int UpdateSound(byte* buf);

static struct eOptionZoom : public xOptions::eOptionInt
{
	eOptionZoom() { Set(2); }
	virtual const char* Name() const { return "zoom"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "fill", "small border", "no border", NULL };
		return values;
	}
	virtual int Order() const { return 1; }
} op_zoom;

static struct eOptionFiltering : public xOptions::eOptionBool
{
	eOptionFiltering() { Set(true); }
	virtual const char* Name() const { return "filtering"; }
	virtual int Order() const { return 2; }
} op_filtering;

static struct eOptionAVTimerSync : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "av timer sync"; }
	virtual int Order() const { return 3; }
} op_av_timer_sync;

static struct eOptionUseSensor : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "use sensor"; }
	virtual int Order() const { return 4; }
} op_use_sensor;

static struct eOptionUseKeyboard : public xOptions::eOptionBool
{
	eOptionUseKeyboard() { Set(true); }
	virtual const char* Name() const { return "use keyboard"; }
	virtual int Order() const { return 5; }
} op_use_keyboard;

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

void Java_app_usp_Emulator_InitRom(JNIEnv* env, jobject obj, jint rom_id, jobject rom_buf)
{
	const byte* rom = (const byte*)env->GetDirectBufferAddress(rom_buf);
	switch(rom_id)
	{
	case 0:	memcpy(sos128_0,rom, sizeof(sos128_0));	break;
	case 1:	memcpy(sos128_1,rom, sizeof(sos128_1));	break;
	case 2:	memcpy(sos48,	rom, sizeof(sos48));	break;
	case 3:	memcpy(service,	rom, sizeof(service));	break;
	case 4:	memcpy(dos513f,	rom, sizeof(dos513f));	break;
	}
}

void Java_app_usp_Emulator_InitFont(JNIEnv* env, jobject obj, jobject fnt_buf)
{
	const byte* fnt = (const byte*)env->GetDirectBufferAddress(fnt_buf);
	memcpy(spxtrm4f, fnt, sizeof(spxtrm4f));
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
	PROFILER_SECTION(u_vid);
	uint16_t* buf = (uint16_t*)env->GetDirectBufferAddress(byte_buffer);
	xPlatform::UpdateScreen(buf);
}
jint Java_app_usp_Emulator_UpdateAudio(JNIEnv* env, jobject obj, jobject byte_buffer)
{
	PROFILER_SECTION(u_aud);
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

void Java_app_usp_Emulator_ProfilerBegin(JNIEnv* env, jobject obj, jint id)
{
	switch(id)
	{
	case 0: PROFILER_BEGIN(pro0);	break;
	case 1: PROFILER_BEGIN(pro1);	break;
	case 2: PROFILER_BEGIN(pro2);	break;
	case 3: PROFILER_BEGIN(pro3);	break;
	}
}
void Java_app_usp_Emulator_ProfilerEnd(JNIEnv* env, jobject obj, jint id)
{
	switch(id)
	{
	case 0: PROFILER_END(pro0);	break;
	case 1: PROFILER_END(pro1);	break;
	case 2: PROFILER_END(pro2);	break;
	case 3: PROFILER_END(pro3);	break;
	}
}

}
//extern "C"

#endif//_ANDROID
