#include "../../std.h"
#include "../../z80/z80.h"
#include "beeper.h"

//=============================================================================
//	eBeeper::eBeeper
//-----------------------------------------------------------------------------
eBeeper::eBeeper(xZ80::eZ80* _cpu) : cpu(_cpu)
{
}
//=============================================================================
//	eBeeper::IoWrite
//-----------------------------------------------------------------------------
void eBeeper::IoWrite(word port, byte v)
{
	if(port & 1)
		return;
	const short vol = 8192;
	short spk = (v & 0x10) ? vol : 0;
	short mic = (v & 0x08) ? vol : 0;
	short mono = spk + mic;
	Update(cpu->T(), mono, mono);
}
