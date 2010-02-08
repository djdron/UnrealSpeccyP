#ifndef __KEMPSTON_JOY_H__
#define __KEMPSTON_JOY_H__

#include "../device.h"

#pragma once

class eKempstonJoy : public eDevice
{
public:
	virtual void Init();
	virtual void Reset();
	virtual void IoRead(word port, byte* v, int tact);
	void OnKey(char key, bool down, bool shift, bool ctrl, bool alt);

	static eDeviceId Id() { return D_KEMPSTON_JOY; }
protected:
	void KeyState(char key, bool down);
	byte state;
};

#endif//__KEMPSTON_JOY_H__
