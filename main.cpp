#include "std.h"
#include "platform/platform.h"
#include "speccy.h"
#include "ula.h"
#include "keyboard.h"
#include "snapshot.h"
#include "sound/device_sound.h"

struct eSpeccyHandler : public xPlatform::eHandler
{
	eSpeccyHandler(eSpeccy* s) : speccy(s)
	{
		sound_dev[0] = speccy->Beeper();
		sound_dev[1] = speccy->AY();
	}
	virtual void OnLoop() { speccy->Update(); }
	virtual void* VideoData() { return speccy->Ula()->Screen(); }
	virtual const char* WindowCaption() { return "UnrealSpeccy portable"; }
	virtual void OnKey(char key, dword flags)
	{
		using namespace xPlatform;
		bool down = flags&KF_DOWN;
		bool shift = flags&KF_SHIFT;
		bool ctrl = flags&KF_CTRL;
		bool alt = flags&KF_ALT;
		speccy->Keyboard()->OnKey(key, down, shift, ctrl, alt);
	}

	virtual int	AudioSources() { return SOUND_DEV_COUNT; }
	virtual void* AudioData(int source) { return sound_dev[source]->AudioData(); }
	virtual dword AudioDataReady(int source) { return sound_dev[source]->AudioDataReady(); }
	virtual void AudioDataUse(int source, dword size) { sound_dev[source]->AudioDataUse(size); }

	eSpeccy* speccy;

	enum { SOUND_DEV_COUNT = 2 };
	eDeviceSound* sound_dev[SOUND_DEV_COUNT];
};

int main(int argc, char* argv[])
{
	eSpeccy* speccy = new eSpeccy;
	speccy->Init();
	speccy->Reset();
	xSnapshot::Load(speccy, "images/vibrate.sna");
	eSpeccyHandler sh(speccy);
	if(!xPlatform::Init(argc, argv))
		return -1;
	xPlatform::Loop();
	xPlatform::Done();
	delete speccy;
	return 0;
}
