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
	cpu = new xZ80::eZ80;
	devices.Add(new eRom, D_ROM);
	devices.Add(new eRam, D_RAM);
	devices.Add(new eUla, D_ULA);
	devices.Init();
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
	cpu->haltpos = 0;
	// INT check separated from main Z80 loop to improve emulation speed
	while(cpu->t < int_len)
	{
		if(cpu->iff1 && cpu->t != cpu->eipos) // int enabled in CPU not issued after EI
		{
			cpu->Int();
		}
		cpu->Step();
		if(cpu->halted)
			break;
	}
	cpu->eipos = -1;
	while(cpu->t < frame_tacts)
	{
		cpu->Step();
		if(nmi_pending)
		{
			nmi_pending--;
			if(cpu->pc >= 0x4000)
			{
				cpu->Nmi();
				nmi_pending = 0;
			}
		}
	}
	cpu->t -= frame_tacts;
	cpu->eipos -= frame_tacts;

	devices.Item(D_ULA)->Update();
}
