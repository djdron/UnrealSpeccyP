#include "std.h"

#include "input.h"

//=============================================================================
//	eKeyboard::IoRead
//-----------------------------------------------------------------------------
void eKeyboard::IoRead(word port, byte* v)
{
	if(!(port & 1)) // port #FE
	{
		if((port >> 8) == 0x7f)
		{
			*v = 0xfe;	//space key
		}
	}
}
