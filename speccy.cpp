#include "std.h"

#include "speccy.h"
#include "device.h"
#include "z80.h"
#include "memory.h"
#include "ula.h"
#include "keyboard.h"
#include "sound/beeper.h"
#include "sound/ay.h"

eSpeccy::eSpeccy() : cpu(NULL), memory(NULL), devices(NULL), frame_tacts(0), int_len(0), nmi_pending(0)
{
}
//=============================================================================
//	eSpeccy::~eSpeccy
//-----------------------------------------------------------------------------
eSpeccy::~eSpeccy()
{
	delete cpu;
	delete memory;
	delete devices;
}
//=============================================================================
//	eSpeccy::Init
//-----------------------------------------------------------------------------
void eSpeccy::Init()
{
	frame_tacts = 71680; //pentagon timing
	int_len = 32;		 //pentagon timing
	memory = new eMemory;
	devices = new eDevices;
	cpu = new xZ80::eZ80(memory, devices, frame_tacts);
	devices->Add(new eRom(memory), D_ROM);
	devices->Add(new eRam(memory), D_RAM);
	devices->Add(new eUla(memory), D_ULA);
	devices->Add(new eKeyboard, D_KEYBOARD);
	devices->Add(new eBeeper(cpu), D_BEEPER);
	devices->Add(new eAY(cpu), D_AY);
}
//=============================================================================
//	eSpeccy::Reset
//-----------------------------------------------------------------------------
void eSpeccy::Reset()
{
	cpu->Reset();
	devices->Reset();
}
//=============================================================================
//	eSpeccy::Update
//-----------------------------------------------------------------------------
void eSpeccy::Update()
{
	Beeper()->StartFrame();
	AY()->StartFrame();
	cpu->Update(int_len, &nmi_pending);
	Ula()->Update();
	Beeper()->EndFrame(cpu->FrameTacts());
	AY()->EndFrame(cpu->FrameTacts());
}
//=============================================================================
//	eSpeccy::Keyboard
//-----------------------------------------------------------------------------
eKeyboard* eSpeccy::Keyboard() const
{
	return (eKeyboard*)devices->Item(D_KEYBOARD);
}
//=============================================================================
//	eSpeccy::Ula
//-----------------------------------------------------------------------------
eUla* eSpeccy::Ula() const
{
	return (eUla*)devices->Item(D_ULA);
}
//=============================================================================
//	eSpeccy::Beeper
//-----------------------------------------------------------------------------
eDeviceSound* eSpeccy::Beeper() const
{
	return (eDeviceSound*)devices->Item(D_BEEPER);
}
//=============================================================================
//	eSpeccy::AY
//-----------------------------------------------------------------------------
eDeviceSound* eSpeccy::AY() const
{
	return (eDeviceSound*)devices->Item(D_AY);
}
