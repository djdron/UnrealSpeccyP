#include "std.h"
#include "platform/platform.h"
#include "speccy.h"
#include "ula.h"

struct eSpeccyHandler : public xPlatform::eHandler
{
	eSpeccyHandler(eSpeccy* s) : speccy(s) {}
	virtual void OnLoop() { speccy->Update(); }
	virtual byte* DrawData() { return ((eUla*)devices.Item(eSpeccy::D_ULA))->Screen(); }
	virtual const char* WindowCaption() { return "UnrealSpeccy portable"; }
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
