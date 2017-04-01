/*
Copyright (c) 2016-2017 Jorge Giner Cordero

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

#include "strdraw.h"
#include "tilengin.h"
#include "bitmaps.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* 
 * These are Unicode codes, sorted so we can do binary search.
 * The index is the tile index in the font.
 */
static const struct unimap {
	int ucode;
	int index;
} s_unimap[] = {
	{ 0xa1, 96 },	/* Inverted exclamation mark */
	{ 0xa9, 97 },	/* Copyright symbol */
	{ 0xbf, 98 },	/* Inverted interrogation mark */
	{ 0xc0, 99 },	/* A grave */
	{ 0xc1, 100 },	/* A acute */
	{ 0xc2, 101 },	/* A circumflex */
	{ 0xc3, 102 },	/* A tilde */
	{ 0xc4, 103 },	/* A diaeresis */
	{ 0xc6, 104 },	/* AE */
	{ 0xc7, 105 },	/* C cedilla */
	{ 0xc8, 106 },	/* E grave */
	{ 0xc9, 107 },	/* E acute */
	{ 0xca, 108 },	/* E circumflex */
	{ 0xcb, 109 },	/* E diaeresis */
	{ 0xcc, 110 },	/* I grave */
	{ 0xcd, 111 },	/* I acute */
	{ 0xce, 112 },	/* I circumflex */
	{ 0xcf, 113 },	/* I diaeresis */
	{ 0xd1, 114 },	/* N tilde */
	{ 0xd2, 115 },	/* O grave */
	{ 0xd3, 116 },	/* O acute */
	{ 0xd4, 117 },	/* O circumflex */
	{ 0xd5, 118 },	/* O tilde */
	{ 0xd6, 119 },	/* O diaeresis */
	{ 0xd9, 120 },	/* U grave */
	{ 0xda, 121 },	/* U acute */
	{ 0xdb, 122 },	/* U circumflex */
	{ 0xdc, 123 },	/* U diaeresis */
	{ 0xdf, 124 },	/* Sharp S */
	{ 0x104, 127 },	/* A ogonek */
	{ 0x106, 128 },	/* C acute */
	{ 0x118, 129 },	/* E ogonek */
	{ 0x141, 130 },	/* L stroke */
	{ 0x143, 131 },	/* N acute */
	{ 0x152, 125 },	/* OE */
	{ 0x15a, 132 },	/* S acute */
	{ 0x178, 126 },	/* Y diaeresis */
	{ 0x179, 133 },	/* Z acute */
	{ 0x17b, 134 },	/* Z dot above */
	{ 0x401, 109 },	/* E diaereis, cyrillic Io */
	{ 0x410, 33 },	/* Cyrillic A */
	{ 0x411, 135 },	/* Cyrillic Be */
	{ 0x412, 34 },	/* Cyrillic Ve */
	{ 0x413, 136 },	/* Cyrillic Ghe */
	{ 0x414, 137 },	/* Cyrillic De */
	{ 0x415, 37 },	/* E, cyrillic Ie */
	{ 0x416, 138 },	/* Cyrillic Zhe */
	{ 0x417, 19 },	/* Cyrillic Ze */
	{ 0x418, 139 },	/* Cyrillic I */
	{ 0x419, 140 },	/* Cyrillic short I */
	{ 0x41a, 43 },	/* K, cyrillic K */
	{ 0x41b, 141 },	/* Cyrillic Ei */
	{ 0x41c, 45 },	/* M, Cyrillic Em */
	{ 0x41d, 40 },	/* H, Cyrillic En */
	{ 0x41e, 47 },	/* O, Cyrillic O */
	{ 0x41f, 142 },	/* Cyrillic Pe */
	{ 0x420, 48 },	/* P, cyrillic Er */
	{ 0x421, 35 },	/* C, cyrillic Es */
	{ 0x422, 52 },	/* T, cyrillic Te */
	{ 0x423, 143 },	/* Cyrillic U */
	{ 0x424, 144 },	/* Cyrillic Ef */
	{ 0x425, 56 },	/* X, cyrillic Ha */
	{ 0x426, 145 },	/* Cyrillic Tse */
	{ 0x427, 146 },	/* Cyrillic Che */
	{ 0x428, 147 },	/* Cyrillic Sha */
	{ 0x429, 148 },	/* Cyrillic Shcha */
	{ 0x42a, 149 },	/* Cyrillic Hard Sign */
	{ 0x42b, 150 },	/* Cyrillic Yeru */
	{ 0x42c, 151 },	/* Cyrillic Soft Sign */
	{ 0x42d, 152 },	/* Cyrillic E */
	{ 0x42e, 153 },	/* Cyrillic Yu */
	{ 0x42f, 154 },	/* Cyrillic Ya */
};

struct bmp *bmp_font0 = NULL;
struct bmp *bmp_font1 = NULL;

static int unicmp(const void *key, const void *elem)
{
	int ucode;
	int ikey;

	ikey = *(const int *) key;
	ucode = ((const struct unimap *) elem)->ucode;
	if (ikey == ucode)
		return 0;
	else if (ikey < ucode)
		return -1;
	else
		return 1;
}

/* Returns the tile index of the unicode code uc.
 * Note that you can pass a character as ' ' if it is ASCII (< 128).
 * If not, maybe the char type is signed and this can lead to error.
 */
int chri(int uc)
{
	struct unimap *pum;

	if (uc < 32) {
		return 0;
	} else if (uc < 128) {
		return uc - 32;
	} else {
		pum = bsearch(&uc, s_unimap, NELEMS(s_unimap),
			      sizeof(*pum), unicmp);
		if (pum == NULL)
			return 0;
		else
			return pum->index;
	}
}

/* 
 * Returns how much bytes to advance in 's', and in *code the Unicode code.
 * If there is a coding error, returns 0 and *code will be '\0'.
 * code can be NULL.
 */
int utf8_next_code(const char *s, int *code)
{
	unsigned char uc;
	int n, i;

	uc = *s;
	n = 0;
	i = 0;
	if (uc == 0) {
		/* nothing */
	} else if (!(uc & 0x80)) {
		n = uc;
		i = 1;
	} else if ((uc & 0xe0) == 0xc0) {
		/* two byte code */
		n = (int) (uc & 0x1f) << 6;
		uc = *(s + 1);
		if ((uc & 0xc0) == 0x80) {
			i = 2;
			n |= uc & 0x3f;
		} else {
			n = 0;
		}
	}

	if (code != NULL)
		*code = n;
	return i;
}

int utf8_strlen(const char *s)
{
	int d, n;

	n = 0;
	while ((d = utf8_next_code(s, NULL)) != 0) {
		s += d;
		n++;
	}

	return n;
}

void draw_str(const char *s, int x, int y, int color)
{
	int ox, uc, d;

	if (x < 0 || y < 0 || x >= TE_FMW || y >= TE_FMH)
		return;

	ox = x;
	while ((d = utf8_next_code(s, &uc)) != 0) {
		if (uc == '\n') {
			x = ox;
			y += 2;
		} else if (x < TE_FMW) {
			te_set_fg_xy(x, y, color, chri(uc));
			x++;
		}
		s += d;
	}
}

static int wordlen(const char *s)
{
	int len, d, uc;

	len = 0;
	while ((d = utf8_next_code(s, &uc)) != 0 && !isspace(uc)) {
		s += d;
		len++;
	}

	return len;
}

/* Draws a word wrapped string.
 * Up to 'len' characters of the string are drawn.
 * If 'len' is negative all the string is drawn.
 * 'w' is the max length of a line. w must be > 0.
 * TODO: this is not efficient.
 */
void draw_str_ww(const char *s, int x, int y, int color, int len, int w)
{
	int ox, sp, wlen, drawn, d, uc;

	if (x < 0 || y < 0 || x >= TE_FMW || y >= TE_FMH)
		return;

	if (len < 0)
		len = INT_MAX;
	if (w <= 0)
		w = 1;

	/* Transform w in the x limit. */
	w = x + w;

	ox = x;
	sp = 1;
	drawn = 0;
	while ((d = utf8_next_code(s, &uc)) != '\0' && drawn < len) {
		if (uc == '\n') {
			sp = 1;
			x = ox;
			y += 2;
		} else if (isspace(uc)) {
			sp = 1;
			if (x > ox) {
				x++;
				if (x >= w) {
					x = ox;
					y += 2;
				}
			}
		} else {
			if (sp && x > ox) {
				wlen = wordlen(s);
				if (x + wlen > w && ox + wlen <= w) {
					x = ox;
					y += 2;
				}
			}
			if (x < TE_FMW) {
				te_set_fg_xy(x, y, color, chri(uc));
			}
			x++;
			if (x >= w) {
				x = ox;
				y += 2;
			}
			sp = 0;
		}
		s += d;
		drawn++;
	}
}

/* Calculates the number of lines a word wrapped string will ocuppy. */
int str_height_ww(const char *s, int x, int w)
{
	int y, ox, sp, wlen, d, uc;

	if (x < 0 || x >= TE_FMW)
		return 0;

	if (w <= 0)
		w = 1;

	/* Transform w in the x limit. */
	w = x + w;

	ox = x;
	sp = 1;
	y = 0;
	while ((d = utf8_next_code(s, &uc)) != '\0') {
		if (uc == '\n') {
			sp = 1;
			x = ox;
			y++;
		} else if (isspace(uc)) {
			sp = 1;
			if (x > ox) {
				x++;
				if (x >= w) {
					x = ox;
					y++;
				}
			}
		} else {
			if (sp && x > ox) {
				wlen = wordlen(s);
				if (x + wlen > w && ox + wlen <= w) {
					x = ox;
					y++;
				}
			}
			x++;
			if (x >= w) {
				x = ox;
				y++;
			}
			sp = 0;
		}
		s += d;
	}

	if (x == ox) {
		return y;
	} else {
		return y + 1;
	}
}

/* Draws an icon specified by the string s.
 * 's' must have at least 4 characters, say "abcd".
 * And they will be drawn this way:
 *
 * ab
 * cd
 *
 */
void draw_icon(const char *s, int x, int y, int color)
{
	if (kassert_fails(s != NULL && strlen(s) >= 4)) {
			return;
	}

	te_set_fg_xy(x, y, color, chri(s[0]));
	te_set_fg_xy(x + 1, y, color, chri(s[1]));
	te_set_fg_xy(x, y + 1, color, chri(s[2]));
	te_set_fg_xy(x + 1, y + 1, color, chri(s[3]));
}

void strdraw_init(void)
{
	register_bitmap(&bmp_font0, "font0", 1, 0x8080);
	register_bitmap(&bmp_font1, "font1", 0, 0);
}
