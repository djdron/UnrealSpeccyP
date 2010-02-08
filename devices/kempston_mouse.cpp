#include "../std.h"
#include "kempston_mouse.h"

void eKempstonMouse::Init() { Reset(); }
void eKempstonMouse::Reset()
{
    x = 31; // assign random different coords (some programs test this)
    y = 85;
    buttons = 0xFF;
}
void eKempstonMouse::IoRead(word port, byte* v, int tact)
{
	if(port & 0x20)
		return;

    port |= 0xFA00; // A13,A15 not used in decoding
    switch(port)
    {
    case 0xFBDF: *v = x; break;
    case 0xFFDF: *v = y; break;
    case 0xFADF: *v = buttons; break;
    }
}
void eKempstonMouse::OnMouseMove(byte _x, byte _y)
{
	x = _x;
	y = _y;
}
void eKempstonMouse::OnMouseButton(byte index, bool state)
{
	byte v = byte(0x01) << index;
	if(state)
		buttons &= ~v;
	else
		buttons |= v;
}
