/*
Copyright (c) 2014, 2015, 2016 Jorge Giner Cordero

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

#ifndef BMP_H
#define BMP_H

#ifndef BMP_LOAD_H
#include "bmp_load.h"
#endif

enum {
	FLIPH = 1,
	FLIPV = 2
};

struct rect {
	int x;
	int y;
	int w;
	int h;
};

#ifdef __cplusplus
extern "C" {
#endif

void reset_clip(int w, int h);

void set_clip(const struct rect *r);

void add_clip(const struct rect *r);

void get_clip(struct rect *r);

void rect_intersect(const struct rect *ra, const struct rect *rb,
		    struct rect *dst);

void set_draw_color(unsigned int color);

void draw_line(struct bmp *dst, int x0, int y0, int x1, int y1);

/*
 * Draws the rect 'src_rect' of 'bmp' in position (dx,dy) of 'dst'.
 * If 'src_rect' is NULL, considers all 'bmp'.
 * Applies the current global clipping.
 */
void draw_bmp(const struct bmp *src, int dx, int dy, struct bmp *dst,
	      const struct rect *src_rect);

/*
 * Same as draw_bmp but if use_key_color is 0, the bitmap is drawn opaque
 * even if src->use_key_color is set.
 * 'transform' is 0, FLIPH, FLIPV or FLIPH | FLIPV.
 */
void draw_bmp_kct(const struct bmp *src, int dx, int dy, struct bmp *dst,
	          const struct rect *src_rect, int use_key_color,
		  int transform);

void bmp_draw_init(void);

#ifdef __cplusplus
}
#endif

#endif
