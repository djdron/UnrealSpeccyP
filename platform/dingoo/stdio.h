/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
