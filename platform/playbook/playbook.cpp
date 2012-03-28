#ifdef USE_SDL
#ifdef __PLAYBOOK__

#include <sys/stat.h>

static struct eFolderInit
{
	eFolderInit()
	{
		mkdir(SDL_DEFAULT_FOLDER, 0777);
	}
} fi;

#endif//__PLAYBOOK__
#endif//USE_SDL
