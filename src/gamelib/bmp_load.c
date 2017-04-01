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

#include "bmp_load.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include <limits.h>
#include <stdint.h>

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>

/*
 * A bitmap file is:
 *
 *	- A BITMAPFILEHEADER.
 *	- A BITMAPCOREHEADER or a BITMAPINFOHEADER or
 *	  BITMAPV4HEADER or a BITMAPV5HEADER.
 *	- bitmap data.
 */

/*
 * We define several BMP headers and symbols here instead of including Windows
 * headers, to make this independent.
 */

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L

typedef int		LONG;
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	DWORD;

#pragma pack(1)
typedef struct {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;
#pragma pack()

typedef struct {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

typedef struct {
	DWORD bcSize;
	WORD  bcWidth;
	WORD  bcHeight;
	WORD  bcPlanes;
	WORD  bcBitCount;
} BITMAPCOREHEADER;

#pragma pack(2)
typedef struct {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
#pragma pack()

typedef struct {
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef int FXPT2DOT30;

typedef struct {
        FXPT2DOT30 ciexyzX;
        FXPT2DOT30 ciexyzY;
        FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct {
        CIEXYZ  ciexyzRed;
        CIEXYZ  ciexyzGreen;
        CIEXYZ  ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct {
        DWORD        bV4Size;
        LONG         bV4Width;
        LONG         bV4Height;
        WORD         bV4Planes;
        WORD         bV4BitCount;
        DWORD        bV4V4Compression;
        DWORD        bV4SizeImage;
        LONG         bV4XPelsPerMeter;
        LONG         bV4YPelsPerMeter;
        DWORD        bV4ClrUsed;
        DWORD        bV4ClrImportant;
        DWORD        bV4RedMask;
        DWORD        bV4GreenMask;
        DWORD        bV4BlueMask;
        DWORD        bV4AlphaMask;
        DWORD        bV4CSType;
        CIEXYZTRIPLE bV4Endpoints;
        DWORD        bV4GammaRed;
        DWORD        bV4GammaGreen;
        DWORD        bV4GammaBlue;
} BITMAPV4HEADER;

typedef struct {
        DWORD        bV5Size;
        LONG         bV5Width;
        LONG         bV5Height;
        WORD         bV5Planes;
        WORD         bV5BitCount;
        DWORD        bV5Compression;
        DWORD        bV5SizeImage;
        LONG         bV5XPelsPerMeter;
        LONG         bV5YPelsPerMeter;
        DWORD        bV5ClrUsed;
        DWORD        bV5ClrImportant;
        DWORD        bV5RedMask;
        DWORD        bV5GreenMask;
        DWORD        bV5BlueMask;
        DWORD        bV5AlphaMask;
        DWORD        bV5CSType;
        CIEXYZTRIPLE bV5Endpoints;
        DWORD        bV5GammaRed;
        DWORD        bV5GammaGreen;
        DWORD        bV5GammaBlue;
        DWORD        bV5Intent;
        DWORD        bV5ProfileData;
        DWORD        bV5ProfileSize;
        DWORD        bV5Reserved;
} BITMAPV5HEADER;

/*
 * Creates a 32 or 8 bpp bitmap of 'width' x 'height'.
 *
 * If 'pal_size' is 0, creates a 32 bpp bitmap.
 * If 'pal_size' in [1..256], creates an 8 bpp bitmap with a palette of
 * 256 colors, but setting palsz to 'pal_size'.
 *
 * The image data and palette data contains garbage.
 *
 * On entry:
 * 	- width asserted > 0
 * 	- height asserted > 0
 * 	- pal_size asserted [0..256]
 *
 * On exit:
 * 	- NULL if no memory or the image is too big or arguments are invalid.
 *	- Only in that case *ecode is valid and can be:
 * 		- E_BMP_MEM: no memory.
 * 		- E_BMP_BIG: bitmap is too big.
 * 		- E_BMP_ERROR: arguments are invalid.
 */
static struct bmp *create_bmp(int width, int height, int pal_size, int *ecode)
{
	struct bmp *im;
	int wxh;

	/* (1) */
	if (kassert_fails(width > 0 && height > 0)) {
		*ecode = E_BMP_ERROR;
		return NULL;
	}

	if (kassert_fails(pal_size >= 0 && pal_size <= 256)) {
		*ecode = E_BMP_ERROR;
		return NULL;
	}

	/* (2) */
	/* Safe by (1): width and height positive. */
	if (imul_overflows_int(width, height)) {
		*ecode = E_BMP_BIG;
		return NULL;
	}

	wxh = width * height;
	if (imul_overflows_int(wxh, 4)) {
		*ecode = E_BMP_BIG;
		return NULL;
	}
	/* )2( */

	im = malloc(sizeof(*im));
	if (im == NULL) {
		*ecode = E_BMP_MEM;
		return NULL;
	}

	im->pal = NULL;
	if (pal_size > 0) {
		/*
		 * Safe: pal_size in range [1..256] by (1).
		 * Multiplied by 4 fits in 15 bits.
		 */
		if ((im->pal = malloc(256 * 4)) == NULL) {
			*ecode = E_BMP_MEM;
			goto freim;
		}
	}

	if (pal_size == 0) {
		/*
		 * Safe by (2).
		 */
		im->pixels = malloc(wxh * 4);
		im->pitch = width * 4;
	} else {
		im->pixels = malloc(wxh);
		im->pitch = width;
	}

	if (im->pixels == NULL) {
		*ecode = E_BMP_MEM;
		goto frepal;
	}

	/*
	 * Safe: short = int in range [1..256] by (1).
	 */
	im->palsz = pal_size;

	im->w = width;
	im->h = height;
	return im;

frepal:	if (im->pal != NULL)
		free(im->pal);
freim:	free(im);
	return NULL;
}

/*
 * Loads the palette of a bitmap from file 'fp', if any, into a 'pal' array
 * with capacity up to 'maxcolors'.
 * Uses 'ih' to discriminate between RGBTRIPLE palettes and RGBQUAD palettes.
 */
static int bmp_load_pal(BITMAPV5HEADER *ih, RGBQUAD *pal, int maxcolors,
			FILE *fp)
{
	RGBTRIPLE cpal[256];
	int ncolors;
	size_t n;

	if (ih->bV5ClrUsed == 0) {
		ncolors = maxcolors;
	} else if (ih->bV5ClrUsed <= (unsigned int) maxcolors) {
		ncolors = ih->bV5ClrUsed;
	} else {
		return -1;
	}

	if (ih->bV5Size != sizeof(BITMAPCOREHEADER)) {
		n = fread(pal, sizeof(*pal) * ncolors, 1, fp);
		return (n == 1) ? 0 : -2;
	}

	/* Core header palette is based on triplets. */
	n = fread(cpal, sizeof(cpal[0]) * ncolors, 1, fp);
	if (n == 0)
		return -2;

	/* Transform to quads. */
	while (ncolors--) {
		pal[ncolors].rgbBlue = cpal[ncolors].rgbtBlue;
		pal[ncolors].rgbGreen = cpal[ncolors].rgbtGreen;
		pal[ncolors].rgbRed = cpal[ncolors].rgbtRed;
		pal[ncolors].rgbReserved = 0;
	}

	return 0;
}

/*
 * Number of bytes in a row padded to 4 bytes.
 * width and bitcnt must be positive.
 * bitcnt must be 1, 4, 8, 16, 24, or 32.
 *
 * Returns -1 on overflow.
 */
static int padded_row_len(int width, int bitcnt)
{
	int64_t n;

	/*
	 * Safe since width is 31 bits and bitcnt is 32 maximum: this
	 * fits in 63 bits.
	 */
	n = (((int64_t) width * bitcnt + 31) / 32) * 4;
	if (n > INT_MAX)
		return -1;
	return (int) n;
}

/*
 * Number of bytes with significant info.
 * width and bitcnt must be positive.
 * bitcnt must be 1, 4, 8, 16, 24, or 32.
 *
 * Returns -1 on overflow.
 */
static int filled_bytes(int width, int bitcnt)
{
	int64_t n;

	/*
	 * Safe since width is 31 bits and bitcnt is 32 maximum: this
	 * fits in 63 bits.
	 */
	n = ((int64_t) width * bitcnt + 7) / 8;
	if (n > INT_MAX)
		return -1;
	return (int) n;
}

/*
 * Loads the BMP pixel data.
 *
 * On entry:
 * 	- nbytes is the number of bytes to load, asserted > 0.
 * 	- fp is the source file.
 *
 * Returns:
 * 	- The pixel data or NULL.
 * 	- In *ecode the error code, if the return value is NULL:
 * 		- E_BMP_MEM: not enough memory.
 * 		- E_BMP_ERROR: not enough data to load from file or
 * 			       nbytes was <= 0.
 */
static unsigned char *bmp_load_data(int nbytes, FILE *fp, int *ecode)
{
	unsigned char *data;

	if (kassert_fails(nbytes > 0)) {
		*ecode = E_BMP_ERROR;
		return NULL;
	}

	data = malloc(nbytes);
	if (data == NULL) {
		*ecode = E_BMP_MEM;
		return NULL;
	}

	if (fread(data, nbytes, 1, fp) == 0) {
		*ecode = E_BMP_ERROR;
		free(data);
		return NULL;
	}

	return data;
}

static void expand_4bpp_rgb(unsigned char *dst, unsigned char *src, int w,
			    int h, int pad)
{
	int i, j, p, s;
	unsigned char k;

	p = 0;
	for (j = 0; j < h; j++) {
		s = 1;
		for (i = 0; i < w; i++) {
			if (s == 1)
				k = src[p] >> 4;
			else
				k = src[p] & 15;
			*dst++ = k;
			s ^= 1;
			p += s;
		}
		p += pad + (s ^ 1);
	}
}

static void expand_4bpp_rle(unsigned char *dst, unsigned char *src, int srclen,
			    int w, int h)
{
	unsigned char c;
	int stat, n, s;
	unsigned char *end;
	unsigned char *dstend;
	int x;
	int pad;

	end = src + srclen;
	dstend = dst + (w * h);
	stat = x = pad = s = 0;
	n = 1;
	while (src != end) {
		switch (stat) {
		case 0:
			if (dst == dstend)
				return;
			c = *src++;
			if (c != 0) {
				n = c;
				s = 1;
				stat = 1;
			} else {
				stat = 2;
			}
			break;
		case 1:
			while (x < w && n--) {
				if (s == 1)
					*dst++ = *src >> 4;
				else
					*dst++ = *src & 15;
				x++;
				s ^= 1;
			}
			src++;
			stat = 0;
			break;
		case 2:
			c = *src;
			if (c == 0) {
				/* End of line. */
				x = 0;
				src++;
				stat = 0;
			} else if (c == 1) {
				/* End of bitmap. */
				return;
			} else if (c == 2) {
				/* Delta. */
				src++;
				stat = 4;
			} else {
				s = 1;
				n = c;
				/* How many bytes we fill? If not multiple
				   of 2 needs padding. */
				pad = ((n * 4 + 7) / 8) & 1;
				stat = 3;
				src++;
			}
			break;
		case 3:
			if (x < w) {
				if (s == 1)
					*dst++ = *src >> 4;
				else
					*dst++ = *src & 15;
				x++;
			}
			s ^= 1;
			src += s;
			if (--n == 0) {
				src += (s ^ 1);
				if (pad)
					stat = 6;
				else
					stat = 0;
			}
			break;
		case 4:
			n = *src++;
			while (x < w && n-- > 0) {
				*dst++ = 0;
				x++;
			}
			stat = 5;
			break;
		case 5:
			n = *src++;
			n *= w;
			while (dst != dstend && n--) {
				*dst++ = 0;
			}
			stat = 0;
			break;
		case 6:
			src++;
			stat = 0;
			break;
		}
	}
}

static struct bmp *load_4bpp_bmp(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	RGBQUAD pal[16];
	int rowlen, filled;
	struct bmp *im;
	unsigned char *data;
	int err;

	im = NULL;
	if (ih->bV5Compression != BI_RGB && ih->bV5Compression != BI_RLE4) {
		/* bad compression for 4bpp bitmap */
		*ecode = E_BMP_COMPRESS_COMPAT;
		goto error;
	}

	err = bmp_load_pal(ih, pal, 16, fp);
	if (err) {
		*ecode = E_BMP_ERROR;
		goto error;
	}

	rowlen = 0;
	if (ih->bV5Compression == BI_RGB) {
		rowlen = padded_row_len(ih->bV5Width, ih->bV5BitCount);
		if (rowlen == -1) {
			*ecode = E_BMP_BIG;
			goto error;
		}
		if (imul_overflows_int(rowlen, abs(ih->bV5Height))) {
			*ecode = E_BMP_BIG;
			goto error;
		}
		data = bmp_load_data(rowlen * abs(ih->bV5Height), fp, ecode);
	} else {
		/* BI_RLE4 */
		data = bmp_load_data(ih->bV5SizeImage, fp, ecode);
	}

	if (data == NULL)
		goto error;

	im = create_bmp(ih->bV5Width, abs(ih->bV5Height), 16, ecode);
	if (im == NULL)
		goto fredat;
	
	memcpy(im->pal, pal, sizeof(pal));
	if (ih->bV5Compression == BI_RGB) {
		filled = filled_bytes(ih->bV5Width, ih->bV5BitCount);
		if (filled == -1) {
			*ecode = E_BMP_BIG;
			goto fredat;
		}
		expand_4bpp_rgb(im->pixels, data, im->w, im->h,
			       	rowlen - filled);
	} else {
		/* BI_RLE4 */
		expand_4bpp_rle(im->pixels, data, ih->bV5SizeImage, im->w,
				im->h);
	}

fredat: free(data);
error:	return im;
}

static void expand_8bpp_rle(unsigned char *dst, unsigned char *src, int srclen,
			    int w, int h)
{
	unsigned char c;
	int stat, n;
	unsigned char *end;
	unsigned char *dstend;
	int x;
	int pad;

	end = src + srclen;
	dstend = dst + (w * h);
	stat = 0;
	x = 0;
	pad = 0;
	n = 1;
	while (src != end) {
		switch (stat) {
		case 0:
			if (dst == dstend)
				return;
			c = *src++;
			if (c != 0) {
				n = c;
				stat = 1;
			} else {
				stat = 2;
			}
			break;
		case 1:
			c = *src;
			while (x < w && n--) {
				*dst++ = c;
				x++;
			}
			src++;
			stat = 0;
			break;
		case 2:
			c = *src;
			if (c == 0) {
				/* End of line. */
				x = 0;
				src++;
				stat = 0;
			} else if (c == 1) {
				/* End of bitmap. */
				return;
			} else if (c == 2) {
				/* Delta. */
				src++;
				stat = 4;
			} else {
				n = c;
				/* How many bytes we fill? If not multiple
				   of 2 needs padding. */
				pad = ((n * 8 + 7) / 8) & 1;
				stat = 3;
				src++;
			}
			break;
		case 3:
			if (x < w) {
				*dst++ = *src++;
				x++;
			}
			if (--n == 0) {
				if (pad)
					stat = 6;
				else
					stat = 0;
			}
			break;
		case 4:
			n = *src++;
			while (x < w && n-- > 0) {
				*dst++ = 0;
				x++;
			}
			stat = 5;
			break;
		case 5:
			n = *src++;
			n *= w;
			while (dst != dstend && n--) {
				*dst++ = 0;
			}
			stat = 0;
			break;
		case 6:
			src++;
			stat = 0;
			break;
		}
	}
}

static void expand_8bpp_rgb(unsigned char *dst, unsigned char *src, int w,
			    int h, int pad)
{
	int tmp;

	while (h--) {
		tmp = w;
		while (w--)
			*dst++ = *src++;
		w = tmp;
		src += pad;
	}
}

static struct bmp *load_8bpp_bmp(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	RGBQUAD pal[256];
	int rowlen, filled;
	struct bmp *im;
	unsigned char *data;
	int err;

	im = NULL;
	if (ih->bV5Compression != BI_RGB && ih->bV5Compression != BI_RLE8) {
		/* bad compression for 8bpp bitmap */
		*ecode = E_BMP_COMPRESS_COMPAT;
		goto error;
	}

	err = bmp_load_pal(ih, pal, 256, fp);
	if (err) {
		*ecode = E_BMP_ERROR;
		goto error;
	}

	rowlen = 0;
	if (ih->bV5Compression == BI_RGB) {
		rowlen = padded_row_len(ih->bV5Width, ih->bV5BitCount);
		if (rowlen == -1) {
			*ecode = E_BMP_BIG;
			goto error;
		}
		if (imul_overflows_int(rowlen, abs(ih->bV5Height))) {
			*ecode = E_BMP_BIG;
			goto error;
		}
		data = bmp_load_data(rowlen * abs(ih->bV5Height), fp, ecode);
	} else {
		/* BI_RLE8 */
		data = bmp_load_data(ih->bV5SizeImage, fp, ecode);
	}

	if (data == NULL)
		goto error;

	im = create_bmp(ih->bV5Width, abs(ih->bV5Height), 256, ecode);
	if (im == NULL)
		goto fredat;
		
	memcpy(im->pal, pal, sizeof(pal));
	if (ih->bV5Compression == BI_RGB) {
		filled = filled_bytes(ih->bV5Width, ih->bV5BitCount);
		if (filled == -1) {
			*ecode = E_BMP_BIG;
			goto fredat;
		}
		expand_8bpp_rgb(im->pixels, data, im->w, im->h,
			       	rowlen - filled);
	} else {
		/* BI_RLE8 */
		expand_8bpp_rle(im->pixels, data, ih->bV5SizeImage, im->w,
				im->h);
	}

fredat:	free(data);
error:	return im;
}

static void expand_1bpp_rgb(unsigned char *dst, unsigned char *src, int w,
			    int h, int pad)
{
	int i, j, p;
	unsigned char s;

	p =  0;
	for (j = 0; j < h; j++) {
		s = 128;
		for (i = 0; i < w; i++) {
			*dst++ = (src[p] & s) != 0;
			s >>= 1;
			if (s == 0) {
				s = 128;
				p++;
			}
		}
		p += pad + (s < 128);	/* TODO: we can always know this. */
	}
}

static struct bmp *load_1bpp_bmp(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	RGBQUAD pal[2];
	int rowlen, filled;
	struct bmp *im;
	unsigned char *data;
	int err;

	im = NULL;
	if (ih->bV5Compression != BI_RGB) {
		/* bad compression for 1bpp bitmap */
		*ecode = E_BMP_COMPRESS_COMPAT;
		goto error;
	}

	err = bmp_load_pal(ih, pal, 2, fp);
	if (err) {
		*ecode = E_BMP_ERROR;
		goto error;
	}

	rowlen = padded_row_len(ih->bV5Width, ih->bV5BitCount);
	if (rowlen == -1) {
		*ecode = E_BMP_BIG;
		goto error;
	}
	if (imul_overflows_int(rowlen, abs(ih->bV5Height))) {
		*ecode = E_BMP_BIG;
		goto error;
	}
	data = bmp_load_data(rowlen * abs(ih->bV5Height), fp, ecode);
	if (data == NULL)
		goto error;

	im = create_bmp(ih->bV5Width, abs(ih->bV5Height), 2, ecode);
	if (im == NULL)
		goto fredat;
	
	memcpy(im->pal, &pal, sizeof(pal));
	filled = filled_bytes(ih->bV5Width, ih->bV5BitCount);
	if (filled == -1) {
		*ecode = E_BMP_BIG;
		goto fredat;
	}

	expand_1bpp_rgb(im->pixels, data, im->w, im->h, rowlen - filled);

fredat:	free(data);
error:	return im;
}

static void expand_24bpp_rgb(unsigned int *dst, unsigned char *src, int w,
			     int h, int pad)
{
	unsigned int pix;
	int tmp;

	kasserta(pad >= 0);

	while (h--) {
		tmp = w;
		while (w--) {
			pix = *src++;
			pix |= *src++ << 8;
			pix |= *src++ << 16;
			*dst++ = pix;
		}
		w = tmp;
		src += pad;
	}
}

static struct bmp *load_24bpp_bmp(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	int rowlen, filled;
	struct bmp *im;
	unsigned char *data;

	im = NULL;
	if (ih->bV5Compression != BI_RGB) {
		/* bad compression for 24bpp bitmap */
		*ecode = E_BMP_COMPRESS_COMPAT;
		goto error;
	}

	rowlen = padded_row_len(ih->bV5Width, ih->bV5BitCount);
	if (rowlen == -1) {
		*ecode = E_BMP_BIG;
		goto error;
	}

	filled = filled_bytes(ih->bV5Width, ih->bV5BitCount);
	if (filled == -1) {
		*ecode = E_BMP_BIG;
		goto error;
	}

	if (imul_overflows_int(rowlen, abs(ih->bV5Height))) {
		*ecode = E_BMP_BIG;
		goto error;
	}

	data = bmp_load_data(rowlen * abs(ih->bV5Height), fp, ecode);
	if (data == NULL)
		goto error;

	im = create_bmp(ih->bV5Width, abs(ih->bV5Height), 0, ecode);
	if (im == NULL)
		goto fredat;
		
	expand_24bpp_rgb((unsigned int *) im->pixels, data, im->w, im->h,
			 rowlen - filled);

fredat:	free(data);
error:	return im;
}

/* Returns the number of bits that are 1 in x. */
static int count_bits_set(unsigned int x)
{
	int b;

	for (b = 0; x != 0; x &= (x - 1))
		b++;

	return b;
}

/**
 * Counts the number of bits that are zero in x from the less significative
 * to the first that is 1.
 */
static int count_left_zero_bits(unsigned int x)
{
	int n;

	for (n = 0; n < 32 && !(x & 1); n++)
		x >>= 1;

	return n;
}

static void calc_shr_mul(unsigned int mask, int *pshr, int *pmul)
{
	int bits, shr, mul;

	bits = count_bits_set(mask);
	shr = count_left_zero_bits(mask);
	if (bits > 8) {
		/*
		 * As bits > 8, bits - 8 is >= 1.
		 * As shr >= 0, shr + bits - 8 >= 1.
		 * As bits + shr is maximum 32, shr + bits - 8 <= 24.
		 * So this is [1..24].
		 */
		shr += bits - 8;
		mul = 256;
	} else {
		mask >>= shr;
		if (mask > 0)
			mul = (255 << 8) / mask;
		else
			mul = 0;
	}
	*pshr = shr;
	*pmul = mul;
}

static unsigned int apply_mask(unsigned int src, unsigned int mask,
			       int shr, int mul)
{
	return (((src & mask) >> shr) * mul) >> 8;
}

static void expand_16bpp_rgb(unsigned int *dst, unsigned char *src, int w,
			     int h, int pad, unsigned int rmask,
			     unsigned int gmask, unsigned int bmask)
{
	int i;
	unsigned int pix;
	unsigned short *wsrc;
	int bshr, gshr, rshr;
	int bmul, gmul, rmul;

	kasserta(pad >= 0);

	calc_shr_mul(bmask, &bshr, &bmul);
	calc_shr_mul(gmask, &gshr, &gmul);
	calc_shr_mul(rmask, &rshr, &rmul);

	wsrc = (unsigned short *) src;
	while (h--) {
		for (i = 0; i < w; i++) {
			pix = apply_mask(*wsrc, bmask, bshr, bmul);
			pix |= apply_mask(*wsrc, gmask, gshr, gmul) << 8;
			pix |= apply_mask(*wsrc, rmask, rshr, rmul) << 16;
			*dst++ = pix;
		       	wsrc++;
		}
		wsrc = (unsigned short *) (((unsigned char *) wsrc) + pad);
	}
}

static struct bmp *load_16bpp_bmp(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	int rowlen, filled;
	struct bmp *im;
	unsigned char *data;
	unsigned int rmask, gmask, bmask;

	im = NULL;
	if (ih->bV5Compression != BI_RGB &&
	    ih->bV5Compression != BI_BITFIELDS) {
		/* bad compression for 24bpp bitmap */
		*ecode = E_BMP_COMPRESS_COMPAT;
		goto error;
	}

	if (ih->bV5Compression == BI_RGB && ih->bV5ClrUsed > 0) {
		/* discard palette if any */
		if (uimul_overflows_int(ih->bV5ClrUsed, 4)) {
			*ecode = E_BMP_BIG;
			goto error;
		}
		if (fseek(fp, ih->bV5ClrUsed * 4, SEEK_CUR) != 0) {
			*ecode = E_BMP_ERROR;
			goto error;
		}
	}

	if (ih->bV5Compression == BI_BITFIELDS) {
		rmask = ih->bV5RedMask;
		gmask = ih->bV5GreenMask;
		bmask = ih->bV5BlueMask;
	} else {
		bmask = 0x1f;
		gmask = 0x3e0;
		rmask = 0x7c00;
	}

	rowlen = padded_row_len(ih->bV5Width, ih->bV5BitCount);
	if (rowlen == -1) {
		*ecode = E_BMP_BIG;
		goto error;
	}

	filled = filled_bytes(ih->bV5Width, ih->bV5BitCount);
	if (filled == -1) {
		*ecode = E_BMP_BIG;
		goto error;
	}

	if (imul_overflows_int(rowlen, abs(ih->bV5Height))) {
		*ecode = E_BMP_BIG;
		goto error;
	}

	data = bmp_load_data(rowlen * abs(ih->bV5Height), fp, ecode);
	if (data == NULL)
		goto error;

	im = create_bmp(ih->bV5Width, abs(ih->bV5Height), 0, ecode);
	if (im == NULL)
		goto fredat;
		
	expand_16bpp_rgb((unsigned int *) im->pixels, data, im->w, im->h,
			 rowlen - filled, rmask, gmask, bmask);

fredat: free(data);
error:	return im;
}

static void expand_32bpp_rgb(unsigned int *dst, unsigned char *src,
			     int w, int h, unsigned int rmask,
			     unsigned int gmask, unsigned int bmask)
{
	int i;
	unsigned int pix;
	unsigned int *dwsrc;
	int bshr, gshr, rshr;
	int bmul, gmul, rmul;

	calc_shr_mul(bmask, &bshr, &bmul);
	calc_shr_mul(gmask, &gshr, &gmul);
	calc_shr_mul(rmask, &rshr, &rmul);

	dwsrc = (unsigned int *) src;
	while (h--) {
		for (i = 0; i < w; i++) {
			pix = apply_mask(*dwsrc, bmask, bshr, bmul);
			pix |= apply_mask(*dwsrc, gmask, gshr, gmul) << 8;
			pix |= apply_mask(*dwsrc, rmask, rshr, rmul) << 16;
			*dst++ = pix;
			dwsrc++;
		}
	}
}

/*
 * Load a 32 bit bitmap or returns NULL on failure.
 *
 * On entry:
 * 	(unchecked)
 * 	- ih->bV5Width is in [1..INT_MAX].
 * 	- ih->bV5BitCount is 32.
 *
 * On exit:
 * 	- *ecode contains the error code only if NULL is returned:
 * 		- E_BMP_MEM
 * 		- E_BMP_BIG
 * 		- E_BMP_ERROR
 * 		- E_BMP COMPRESS_COMPAT
 * 		
 */
static struct bmp *load_32bpp_bmp(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	int rowlen;
	struct bmp *im;
	unsigned char *data;
	unsigned int rmask, gmask, bmask;

	im = NULL;
	if (ih->bV5Compression != BI_RGB &&
	    ih->bV5Compression != BI_BITFIELDS) {
		/* bad compression for 32bpp bitmap */
		*ecode = E_BMP_COMPRESS_COMPAT;
		goto error;
	}

	if (ih->bV5Compression == BI_RGB && ih->bV5ClrUsed > 0) {
		/* Discard palette if any */
		if (uimul_overflows_int(ih->bV5ClrUsed, 4)) {
			*ecode = E_BMP_BIG;
			goto error;
		}
		if (fseek(fp, ih->bV5ClrUsed * 4, SEEK_CUR) != 0) {
			*ecode = E_BMP_ERROR;
			goto error;
		}
	}

	if (ih->bV5Compression == BI_BITFIELDS) {
		rmask = ih->bV5RedMask;
		gmask = ih->bV5GreenMask;
		bmask = ih->bV5BlueMask;
	} else {
		bmask = 0xff;
		gmask = 0xff00;
		rmask = 0xff0000;
	}

	/*
	 * There is no padding in a 32bpp bitmap.
	 */
	if (imul_overflows_int(ih->bV5Width, 4)) {
		*ecode = E_BMP_BIG;
		goto error;
	}
	rowlen = ih->bV5Width * 4;
	if (imul_overflows_int(rowlen, abs(ih->bV5Height))) {
		*ecode = E_BMP_BIG;
		goto error;
	}
	data = bmp_load_data(rowlen * abs(ih->bV5Height), fp, ecode);
	if (data == NULL)
		goto error;

	im = create_bmp(ih->bV5Width, abs(ih->bV5Height), 0, ecode);
	if (im == NULL)
		goto fredat;

	expand_32bpp_rgb((unsigned int *) im->pixels, data, im->w, im->h,
			 rmask, gmask, bmask);

fredat:	free(data);
error:	return im;
}

static void vflip_bmp32(struct bmp *im)
{
	unsigned int *ra, *rb, *nra, *nrb;
	unsigned int tmp;
	int i, h2, w;

	w = im->w;
	ra = (unsigned int *) im->pixels;
	rb = ra + w * (im->h - 1);
	for (h2 = im->h / 2; h2 > 0; h2--) {
		nra = ra;
		nrb = rb;
		for (i = 0; i < w; i++) {
			tmp = *nra;
			*nra = *nrb;
			*nrb = tmp;
			nra++;
			nrb++;
		}
		ra = nra;
		rb -= w;
	}
}

static void vflip_bmp8(struct bmp *im)
{
	unsigned char *ra, *rb, *nra, *nrb;
	unsigned char tmp;
	int i, h2, w;

	w = im->w;
	ra = im->pixels;
	rb = ra + w * (im->h - 1);
	for (h2 = im->h / 2; h2 > 0; h2--) {
		nra = ra;
		nrb = rb;
		for (i = 0; i < w; i++) {
			tmp = *nra;
			*nra = *nrb;
			*nrb = tmp;
			nra++;
			nrb++;
		}
		ra = nra;
		rb -= w;
	}
}

/* Given a bitmap, flips it in vertical. */
static void vflip_bmp(struct bmp *im)
{
	if (im->pal == NULL)
		vflip_bmp32(im);
	else
		vflip_bmp8(im);
}

/*
 * On exit:
 * 	- *ecode contains the error code only if NULL is returned:
 * 		- E_BMP_MEM
 * 		- E_BMP_BIG
 * 		- E_BMP_ERROR
 * 		- E_BMP COMPRESS_COMPAT
		- E_BMP_BPP_SUPPORT
 */
static struct bmp *bmp_load_on_type(BITMAPV5HEADER *ih, FILE *fp, int *ecode)
{
	struct bmp *im;

	im = NULL;
	if (ih->bV5Compression == BI_PNG || ih->bV5Compression == BI_JPEG) {
		*ecode = E_BMP_COMPRESS_SUPPORT;
		return NULL;
	}

	if (ih->bV5BitCount == 1) {
		im = load_1bpp_bmp(ih, fp, ecode);
	} else if (ih->bV5BitCount == 4) {
		im = load_4bpp_bmp(ih, fp, ecode);
	} else if (ih->bV5BitCount == 8) {
		im = load_8bpp_bmp(ih, fp, ecode);
	} else if (ih->bV5BitCount == 16) {
		im = load_16bpp_bmp(ih, fp, ecode);
	} else if (ih->bV5BitCount == 24) {
		im = load_24bpp_bmp(ih, fp, ecode);
	} else if (ih->bV5BitCount == 32) {
		im = load_32bpp_bmp(ih, fp, ecode);
	} else {
		*ecode = E_BMP_BPP_SUPPORT;
		return NULL;
	}

	if (im == NULL)
		return NULL;

	if (ih->bV5Height > 0)
		vflip_bmp(im);

	return im;
}

struct bmp *load_bmp_fp(FILE *fp, int *ecode)
{
	int ec;
	size_t n;
	struct bmp *r;
	BITMAPFILEHEADER fh;
	BITMAPV5HEADER ih;
	BITMAPCOREHEADER ch;

	r = NULL;
	if (kassert_fails(fp != NULL)) {
		ec = E_BMP_ERROR;
		goto end; 
	}

	n = fread(&fh, sizeof(fh), 1, fp);
	if (n == 0) {
		ec = E_BMP_ERROR;
		goto end;
	}

	if (fh.bfType != 0x4d42) {
		ec = E_BMP_ERROR;
		goto end;
	}

	n = fread(&ch, sizeof(ch), 1, fp);
	if (n == 0) {
		ec = E_BMP_ERROR;
		goto end;
	}

	memset(&ih, 0, sizeof(ih));

	/*
	 * Safe: every comparison of bcSize and sizeof: unsigned.
	 */
	if (ch.bcSize == sizeof(ch)) {
		/*
		 * If we are a coreheader, set values of a v5 header.
		 */
		ih.bV5Size = ch.bcSize;		/* Safe: 32u = 32u */
		ih.bV5Width = ch.bcWidth; 	/* Safe: 32 = 16u */
		ih.bV5Height = ch.bcHeight;	/* Safe: 32 = 16u */
		ih.bV5Planes = ch.bcPlanes;	/* Safe: 16u = 16u */
		ih.bV5BitCount = ch.bcBitCount;	/* Safe: 16u = 16u */
	}
       	else if (ch.bcSize == sizeof(BITMAPINFOHEADER) ||
	         ch.bcSize == sizeof(BITMAPV4HEADER) ||
	         ch.bcSize == sizeof(BITMAPV5HEADER))
       	{
		/*
		 * We are an info header or superior (v4, v5).
		 * Set the base part and read the rest.
		 */
		memcpy(&ih, &ch, sizeof(ch));

		/*
		 * Safe: bV5Size has the size of a known header.
		 */
		n = fread(((unsigned char *) &ih) + sizeof(ch),
			  ih.bV5Size - sizeof(ch), 1, fp);
		if (n == 0) {
			ec = E_BMP_ERROR;
			goto end;
		}
	} else {
		ec = E_BMP_ERROR;
		goto end;
	}

	if (ih.bV5Width <= 0 || ih.bV5Height == 0) {
		ec = E_BMP_ERROR;
		goto end;
	}

	if (ih.bV5Size == sizeof(BITMAPINFOHEADER) &&
	    ih.bV5Compression == BI_BITFIELDS)
	{
		/* Read 3 color masks. */
		n = fread(((unsigned char *) &ih) + sizeof(BITMAPINFOHEADER),
			  sizeof(ih.bV5RedMask) * 3, 1, fp);
		if (n == 0) {
			ec = E_BMP_ERROR;
			goto end;
		}
	}

	r = bmp_load_on_type(&ih, fp, &ec);

end:	if (ecode != NULL)
		*ecode = ec;
	return r;
}

void free_bmp(struct bmp *bmp, int free_pal)
{
	if (bmp == NULL)
		return;

	if (bmp->pixels != NULL) {
		free(bmp->pixels);
		bmp->pixels = NULL;
	}
	if (free_pal && bmp->pal != NULL) {
		free(bmp->pal);
		bmp->pal = NULL;
	}
	free(bmp);
}
