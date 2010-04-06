#include "../../std.h"
#include "kempston_joy.h"

void eKempstonJoy::Init() { Reset(); }
void eKempstonJoy::Reset() { state = 0; }

void eKempstonJoy::IoRead(word port, byte* v, int tact)
{
	if(port & 0x20)
		return;

	// skip kempston mouse ports
    port |= 0xFA00; // A13,A15 not used in decoding
    if((port == 0xFADF || port == 0xFBDF || port == 0xFFDF))
    	return;

	*v = state;
}

struct eButton
{
	char key;
	byte bit;
};
enum { BUTTONS_COUNT = 5 };
static const eButton buttons[BUTTONS_COUNT] =
{
	{'r',	0x01 },
	{'l',	0x02 },
	{'d',	0x04 },
	{'u',   0x08 },
	{'f',	0x10 },
};
void eKempstonJoy::OnKey(char _key, bool _down)
{
	KeyState(_key, _down);
}
void eKempstonJoy::KeyState(char _key, bool _down)
{
	for(int i = 0; i < BUTTONS_COUNT; ++i)
	{
		const eButton& b = buttons[i];
		if(_key == b.key)
		{
			if(_down)
				state |= b.bit;
			else
				state &= ~b.bit;
			break;
		}
	}
}
