#ifndef __TYPES_H__
#define __TYPES_H__

#pragma once

typedef	unsigned long long int qword;
typedef	unsigned int		dword;
typedef unsigned short		word;
typedef unsigned char		byte;

enum eZeroValue { ZERO };

#ifndef NULL
#define NULL 0
#endif

template<class T> void SAFE_DELETE(T*& p) { if(p) delete p; p = NULL;}
#define SAFE_CALL(p) if(p) p

#endif//__TYPES_H__
