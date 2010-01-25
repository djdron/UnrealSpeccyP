#include "../std.h"
#include "platform.h"

namespace xPlatform
{

static eHandler* handler = NULL;

eHandler::eHandler()
{
	assert(!handler);
	handler = this;
}
eHandler::~eHandler()
{
	handler = NULL;
}
eHandler* Handler() { return handler; }

}
//namespace xPlatform
