#ifndef	__SPECCY_H__
#define	__SPECCY_H__

#pragma once

namespace xZ80 { class eZ80; }
class eMemory;
class eDevices;
class eKeyboard;
class eUla;

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

	enum eDevice { D_ROM, D_RAM, D_ULA, D_KEYBOARD };
protected:
	dword	frame_tacts;	// t-states per frame
	dword	int_len;		// length of INT signal (for Z80)
	int		nmi_pending;
	xZ80::eZ80* cpu;
	eDevices* devices;
	eMemory* memory;
};

#endif//__SPECCY_H__
