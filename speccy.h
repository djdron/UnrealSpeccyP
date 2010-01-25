#ifndef	__SPECCY_H__
#define	__SPECCY_H__

#pragma once

namespace xZ80 { class eZ80; }
class eMemory;
class eDevices;
class eKeyboard;
class eUla;
class eDeviceSound;

//*****************************************************************************
//	eSpeccy
//-----------------------------------------------------------------------------
class eSpeccy
{
public:
	eSpeccy();
	virtual ~eSpeccy();
	void Init();
	void Reset();
	void Load() {}
	void Update();

	xZ80::eZ80* CPU() const { return cpu; }
	eKeyboard* Keyboard() const;
	eUla* Ula() const;
	eDeviceSound* Beeper() const;
	eDeviceSound* AY() const;

	enum eDevice { D_ROM, D_RAM, D_ULA, D_KEYBOARD, D_BEEPER, D_AY };
protected:
	xZ80::eZ80* cpu;
	eMemory* memory;
	eDevices* devices;

	dword	frame_tacts;	// t-states per frame
	dword	int_len;		// length of INT signal (for Z80)
	int		nmi_pending;
};

#endif//__SPECCY_H__
