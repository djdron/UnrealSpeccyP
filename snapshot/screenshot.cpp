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

#include "../std.h"
#include "../speccy.h"

#include "../tools/libpng/png.h"

namespace xScreenshot
{

bool Store(eSpeccy* speccy, const char* file)
{
	FILE* png_file = fopen(file, "wb");
	if(!png_file)
		return false;

	png_struct    *png_ptr = NULL;
	png_info      *info_ptr = NULL;
	png_byte      *png_pixels = NULL;
	png_byte      **row_pointers = NULL;
	png_byte      *pix_ptr = NULL;
	png_uint_32   row_bytes;

	int width = 320, height = 240, bit_depth = 16, color_type = PNG_COLOR_TYPE_RGB;

	/* prepare the standard PNG structures */
	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		return false;
	}
	info_ptr = png_create_info_struct (png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
		return false;
	}

	/* setjmp() must be called in every function that calls a PNG-reading libpng function */
	if (setjmp (png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
		return false;
	}

	/* initialize the png structure */
	png_init_io (png_ptr, png_file);

	/* we're going to write more or less the same PNG as the input file */
	png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	/* write the file header information */
	png_write_info (png_ptr, info_ptr);

	/* if needed we will allocate memory for an new array of row-pointers */
	if (row_pointers == (unsigned char**) NULL)
	{
		if ((row_pointers = (png_byte **) malloc (height * sizeof (png_bytep))) == NULL)
		{
			png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
			return false;
		}
	}

	/* set the individual row_pointers to point at the correct offsets */
	for(int i = 0; i < (height); i++)
		row_pointers[i] = png_pixels + i * row_bytes;

	/* write out the entire image data in one call */
	png_write_image (png_ptr, row_pointers);

	/* write the additional chuncks to the PNG file (not really needed) */
	png_write_end (png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct (&png_ptr, (png_infopp) NULL);

	if (row_pointers != (unsigned char**) NULL)
		free (row_pointers);
	if (png_pixels != (unsigned char*) NULL)
		free (png_pixels);
	return true;
}

}
//namespace xScreenshot
