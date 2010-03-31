#include "log.h"

#ifdef USE_LOG

#include "../std.h"
#include "io.h"

namespace xLog
{

static char log_path[xIo::MAX_PATH_LEN] = { 0 };
void SetLogPath(const char* _path)
{
	strcpy(log_path, _path);
}
static FILE* log_file = NULL;
static void Close()
{
	_LOG("closing log..\n");
	fclose(log_file);
	log_file = NULL;
}
static void Open()
{
	char log_name[xIo::MAX_PATH_LEN];
	strcpy(log_name, log_path);
	strcat(log_name, "usp.log");
	log_file = fopen(log_name, "w");
	assert(log_file);
	atexit(Close);
}
void Write(const char* text)
{
	if(!log_file)
		Open();
	fputs(text, log_file);
	fflush(log_file);
}

}
//namespace xLog

#endif//USE_LOG
