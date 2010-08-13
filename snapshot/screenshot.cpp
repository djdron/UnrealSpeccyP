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

#include "../platform/platform.h"

#ifdef USE_PNG

#include "../tools/libpng/png.h"

namespace xScreenshot
{

static bool StorePNG(eSpeccy* speccy, FILE* png_file)
{
	int width = 320, height = 240, bit_depth = 8, color_type = PNG_COLOR_TYPE_RGB;
	png_uint_32 row_bytes = width*3;// 3 bytes (R, G, B) per pixel

	png_byte* png_pixels = new byte[row_bytes*height];
	png_byte* p = png_pixels;
	byte* data = (byte*)xPlatform::Handler()->VideoData();
	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x)
		{
			const byte brightness = 200;
			const byte bright_intensity = 55;
			byte c = *data++;
			byte i = c&8 ? brightness + bright_intensity : brightness;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			*p++ = r;
			*p++ = g;
			*p++ = b;
		}
	}

	png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		SAFE_DELETE_ARRAY(png_pixels);
		return false;
	}
	png_info* info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		SAFE_DELETE_ARRAY(png_pixels);
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return false;
	}
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		SAFE_DELETE_ARRAY(png_pixels);
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return false;
	}
	png_init_io(png_ptr, png_file);
	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_byte** row_pointers = new png_byte*[height];
	for(int i = 0; i < height; i++)
		row_pointers[i] = png_pixels + i*row_bytes;
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	SAFE_DELETE_ARRAY(row_pointers);
	SAFE_DELETE_ARRAY(png_pixels);
	return true;
}

bool Store(eSpeccy* speccy, const char* file)
{
	FILE* f = fopen(file, "wb");
	if(!f)
		return false;
	bool ok = StorePNG(speccy, f);
	fclose(f);
	return ok;
}

}
//namespace xScreenshot

#endif//USE_PNG
