#include "../std.h"
#include "io.h"

namespace xIo
{

static char resource_path[MAX_PATH_LEN] = { 0 };
//=============================================================================
//	SetResourcePath
//-----------------------------------------------------------------------------
void SetResourcePath(const char* _path)
{
	strcpy(resource_path, _path);
}
static char buf[MAX_PATH_LEN];
//=============================================================================
//	ResourcePath
//-----------------------------------------------------------------------------
const char* ResourcePath(const char* _path)
{
	strcpy(buf, resource_path);
	strcat(buf, _path);
	return buf;
}

}
//namespace xIo
