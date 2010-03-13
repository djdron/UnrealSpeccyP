#ifndef __BEEPER_H__
#define __BEEPER_H__

#include "device_sound.h"

#pragma once

//=============================================================================
//	eBeeper
//-----------------------------------------------------------------------------
class eBeeper : public eDeviceSound
{
public:
	virtual void IoWrite(word port, byte v, int tact);
	static eDeviceId Id() { return D_BEEPER; }
};

#endif//__BEEPER_H__
