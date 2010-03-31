#ifndef __LOG_H__
#define __LOG_H__

#include "platform.h"

namespace xLog
{

#ifdef USE_LOG
void SetLogPath(const char* path);
void Write(const char* text);
#else//USE_LOG
inline void SetLogPath(const char* path) {}
#endif//USE_LOG

}
//namespace xLog

#ifdef USE_LOG
#define _LOG(a) xLog::Write(a)
#else//USE_LOG
#define _LOG(a)
#endif//USE_LOG

#endif//__LOG_H__
