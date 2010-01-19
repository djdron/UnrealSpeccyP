#include "std.h"
#include "platform/platform.h"
#include "speccy.h"
#include "ula.h"
#include "keyboard.h"

struct eSpeccyHandler : public xPlatform::eHandler
{
	eSpeccyHandler(eSpeccy* s) : speccy(s) {}
	virtual void OnLoop() { speccy->Update(); }
	virtual byte* DrawData() { return ((eUla*)devices.Item(eSpeccy::D_ULA))->Screen(); }
	virtual const char* WindowCaption() { return "UnrealSpeccy portable"; }
	virtual void OnKey(char key, dword flags)
	{
		using namespace xPlatform;
		eKeyboard* k = (eKeyboard*)devices.Item(eSpeccy::D_KEYBOARD);
		bool down = flags&KF_DOWN;
		bool shift = flags&KF_SHIFT;
		bool ctrl = flags&KF_CTRL;
		bool alt = flags&KF_ALT;
		k->OnKey(key, down, shift, ctrl, alt);
	}
	eSpeccy* speccy;
};

int main(int argc, char* argv[])
{
	eSpeccy* speccy = new eSpeccy;
	speccy->Init();
	speccy->Reset();
	eSpeccyHandler sh(speccy);
	if(!xPlatform::Init(argc, argv, &sh))
		return -1;
	xPlatform::Loop();
	xPlatform::Done();
	delete speccy;
	return 0;
}
