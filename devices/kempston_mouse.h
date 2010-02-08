#ifndef __KEMPSTON_MOUSE_H__
#define __KEMPSTON_MOUSE_H__

#include "device.h"

#pragma once

class eKempstonMouse : public eDevice
{
public:
	virtual void Init();
	virtual void Reset();
	virtual void IoRead(word port, byte* v, int tact);
	void OnMouseMove(byte x, byte y);
	void OnMouseButton(byte index, bool state);

	static eDeviceId Id() { return D_KEMPSTON_MOUSE; }
protected:
	byte x, y, buttons;
};

#endif//__KEMPSTON_MOUSE_H__
