#include "../../std.h"
#include "beeper.h"

//=============================================================================
//	eBeeper::IoWrite
//-----------------------------------------------------------------------------
void eBeeper::IoWrite(word port, byte v, int tact)
{
	if(port & 1)
		return;
	const short vol = 8192;
	short spk = (v & 0x10) ? vol : 0;
	short mic = (v & 0x08) ? vol : 0;
	short mono = spk + mic;
	Update(tact, mono, mono);
}
