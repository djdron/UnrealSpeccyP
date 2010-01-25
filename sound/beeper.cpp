#include "../std.h"
#include "beeper.h"
#include "../z80.h"

eBeeper::eBeeper(xZ80::eZ80* _cpu) : cpu(_cpu)
{
}

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
