#ifndef	__INPUT_H__
#define	__INPUT_H__

#include "device.h"

#pragma once

//*****************************************************************************
//	eKeyboard
//-----------------------------------------------------------------------------
class eKeyboard : public eDevice
{
	virtual void Init() {}
	virtual void Reset() {}
	virtual void IoRead(word port, byte* v);
};

#endif//__INPUT_H__
