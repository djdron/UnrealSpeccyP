#include "std.h"

#include "speccy.h"
#include "device.h"
#include "z80.h"
#include "memory.h"
#include "ula.h"

//=============================================================================
//	eSpeccy::Init
//-----------------------------------------------------------------------------
void eSpeccy::Init()
{
	frame_tacts = 71680; //pentagon timing
	int_len = 32;		 //pentagon timing
	cpu = new xZ80::eZ80(frame_tacts);
	devices.Add(new eRom, D_ROM);
	devices.Add(new eRam, D_RAM);
	devices.Add(new eUla, D_ULA);
}
//=============================================================================
//	eSpeccy::Reset
//-----------------------------------------------------------------------------
void eSpeccy::Reset()
{
	cpu->Reset();
	devices.Reset();
}
//=============================================================================
//	eSpeccy::Update
//-----------------------------------------------------------------------------
void eSpeccy::Update()
{
	cpu->Update(int_len, &nmi_pending);
	devices.Item(D_ULA)->Update();
}
