/*
Copyright (c) 2014-2017 Jorge Giner Cordero

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BMP_LOAD_H
#define BMP_LOAD_H

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

enum {
	E_BMP_OK,
	E_BMP_MEM,
	E_BMP_BIG,
	E_BMP_COMPRESS_COMPAT,
	E_BMP_COMPRESS_SUPPORT,
	E_BMP_BPP_SUPPORT,
	E_BMP_ERROR
};

/**
 * A bitmap.
 *
 * If pal is not NULL, then palsz is in range [1..256] and data contains
 * indexes into pal, which contains 256 32-bit xrgb colors.
 *
 * If pal is NULL, palsz is zero, and data should be considered as
 * (unsigned int *) containing 32 bit xrgb colors.
 *
 * 'pitch' is the length of a row of pixels in bytes; that is, you have
 * 'w' pixels, but at the end of each row there can be some padding bytes.
 * 
 * 'key_color' is a 32 bit xrgb color used for transparency.
 * 'use_key_color' is true if 'key'color' should be used as the transparent
 * color for drawing algorithms.
 */
struct bmp {
	unsigned char *pixels;
	int w, h;
	int pitch;
	unsigned int *pal;
	short palsz;
	signed char use_key_color;
	unsigned int key_color;
};

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Loads a Windows BMP from fp.
 *
 * Returns NULL on error.
 * In that case *ecode will contain the error code.
 * ecode can be NULL.
 *
 * Allowed formats are: 1 bpp, 4 bpp, 4 bpp rle, 8 bpp, 8 bpp rle, 16 bpp,
 * 24 bpp, 32 bpp.
 *
 * JPEG or PNG embedded formats not supported.
 */
struct bmp *load_bmp_fp(FILE *fp, int *ecode);

/*
 * Frees a bmp and all its data.
 * Call with false 'free_pal' to not free the palette in case it is shared
 * among other bmps.
 */
void free_bmp(struct bmp *bmp, int free_pal);

#ifdef __cplusplus
}
#endif

#endif
