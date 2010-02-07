#include "std.h"
#include "platform/platform.h"
#include "speccy.h"
#include "devices/ula.h"
#include "devices/keyboard.h"
#include "devices/kempston_joy.h"
#include "devices/sound/ay.h"
#include "devices/sound/beeper.h"
#include "devices/fdd/wd93.h"
#include "snapshot.h"

static struct eSpeccyHandler : public xPlatform::eHandler
{
	eSpeccyHandler() : video_paused(false)
	{
		speccy = new eSpeccy;
		sound_dev[0] = speccy->Device<eBeeper>();
		sound_dev[1] = speccy->Device<eAY>();
	}
	~eSpeccyHandler()
	{
		delete speccy;
	}
	virtual void OnLoop() { if(!video_paused) speccy->Update(); }
	virtual void* VideoData() { return speccy->Device<eUla>()->Screen(); }
	virtual const char* WindowCaption() { return "UnrealSpeccy portable"; }
	virtual void OnKey(char key, dword flags)
	{
		using namespace xPlatform;
		bool down = flags&KF_DOWN;
		bool shift = flags&KF_SHIFT;
		bool ctrl = flags&KF_CTRL;
		bool alt = flags&KF_ALT;
		speccy->Device<eKeyboard>()->OnKey(key, down, shift, ctrl, alt);
		speccy->Device<eKempstonJoy>()->OnKey(key, down, shift, ctrl, alt);
	}
	virtual void OnOpenFile(const char* name)
	{
		int l = strlen(name);
		if(l > 3)
		{
			const char* n = name + l - 4;
			if(!strcmp(n, ".trd") || !strcmp(n, ".scl"))
				speccy->Device<eWD1793>()->OpenImage(0, name);
			else if(!strcmp(n, ".sna"))
				xSnapshot::Load(speccy, name);
		}
	}
	virtual void OnReset() { speccy->Reset(); }
	virtual int	AudioSources() { return SOUND_DEV_COUNT; }
	virtual void* AudioData(int source) { return sound_dev[source]->AudioData(); }
	virtual dword AudioDataReady(int source) { return sound_dev[source]->AudioDataReady(); }
	virtual void AudioDataUse(int source, dword size) { sound_dev[source]->AudioDataUse(size); }
	virtual void VideoPaused(bool paused) {	video_paused = paused; }

	eSpeccy* speccy;
	bool video_paused;

	enum { SOUND_DEV_COUNT = 2 };
	eDeviceSound* sound_dev[SOUND_DEV_COUNT];
} sh;

// see platform-specific files for main() function
