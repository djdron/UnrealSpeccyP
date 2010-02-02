#ifndef	__SPECCY_H__
#define	__SPECCY_H__

#include "devices/device.h"

#pragma once

namespace xZ80 { class eZ80; }
class eMemory;

//*****************************************************************************
//	eSpeccy
//-----------------------------------------------------------------------------
class eSpeccy
{
public:
	eSpeccy();
	virtual ~eSpeccy();

	void Reset();
	void Update();

	xZ80::eZ80* CPU() const { return cpu; }

	template<class T> T* Device() const { return devices.Get<T>(); }

	const dword T() const { return t_states; }

protected:
	xZ80::eZ80* cpu;
	eMemory* memory;
	eDevices devices;

	int		frame_tacts;	// t-states per frame
	int		int_len;		// length of INT signal (for Z80)
	int		nmi_pending;
	int		t_states;
};

#endif//__SPECCY_H__
