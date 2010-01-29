#ifndef __BEEPER_H__
#define __BEEPER_H__

#include "device_sound.h"

#pragma once

namespace xZ80 { class eZ80; }

//=============================================================================
//	eBeeper
//-----------------------------------------------------------------------------
class eBeeper : public eDeviceSound
{
public:
	eBeeper(xZ80::eZ80* cpu);
	virtual void IoWrite(word port, byte v);
protected:
	xZ80::eZ80* cpu;
};

#endif//__BEEPER_H__
