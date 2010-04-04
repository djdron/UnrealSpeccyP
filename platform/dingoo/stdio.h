#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdlib.h>

#pragma once

#ifdef _DINGOO

struct FILE;

#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */

extern "C"
{
FILE*	fsys_fopen(const char*, const char*);
size_t	fsys_fread(void*, size_t, size_t, FILE*);
int		fsys_fwrite(const void*, size_t, size_t, FILE*);
int		fsys_fclose(FILE*);
int		fsys_fseek(FILE*, long, int);
long	fsys_ftell(FILE*);

int		sprintf(char*, const char*, ...);
}
//extern "C"

#define fopen	fsys_fopen
#define fread	fsys_fread
#define fwrite	fsys_fwrite
#define fclose	fsys_fclose
#define fseek	fsys_fseek
#define ftell	fsys_ftell

#endif//_DINGOO

#endif//__STDIO_H__
