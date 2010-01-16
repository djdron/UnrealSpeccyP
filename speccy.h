#ifndef	__SPECCY_H__
#define	__SPECCY_H__

#pragma once

namespace xZ80 { class eZ80; }

//*****************************************************************************
//	eSpeccy
//-----------------------------------------------------------------------------
class eSpeccy
{
public:
	eSpeccy() : frame_tacts(0), int_len(0), nmi_pending(0), cpu(NULL) {}
	void Init();
	void Reset();
	void Load() {}
	void Update();

	enum eDevice { D_ROM, D_RAM, D_ULA };
protected:
	dword	frame_tacts;	// t-states per frame
	dword	int_len;		// length of INT signal (for Z80)
	int		nmi_pending;

	xZ80::eZ80* cpu;
};

#endif//__SPECCY_H__
