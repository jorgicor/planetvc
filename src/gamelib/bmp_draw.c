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

#include "bmp.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include <string.h>

static struct rect s_clip;

static unsigned int s_draw_color;
static unsigned char s_draw_pal_color;

/*
 * Intersects 'ra' and 'rb' into 'dst'. 'ra' or 'rb' can be 'dst'.
 */
void rect_intersect(const struct rect *ra, const struct rect *rb,
		    struct rect *dst)
{
	int x, y, x2, y2, xt, yt;

	if (ra == NULL || rb == NULL) {
		if (dst != NULL) {
			memset(dst, 0, sizeof(*dst));
		}
		return;
	}

	if (ra->x >= rb->x)
		x = ra->x;
	else
		x = rb->x;

	if (ra->y >= rb->y)
		y = ra->y;
	else
		y = rb->y;

	x2 = ra->x + ra->w;
	xt = rb->x + rb->w;
	if (x2 > xt)
		x2 = xt;

	y2 = ra->y + ra->h;
	yt = rb->y + rb->h;
	if (y2 > yt)
		y2 = yt;

	if (x >= x2 || y >= y2) {
		memset(dst, 0, sizeof(*dst));
		return;
	}

	dst->x = x;
	dst->y = y;
	dst->w = x2 - x;
	dst->h = y2 - y;
}

/*
 * Resets the current clipping rect to be { 0, 0, w, h }.
 */
void reset_clip(int w, int h)
{
	s_clip.x = 0;
	s_clip.y = 0;
	s_clip.w = w;
	s_clip.h = h;
}

/*
 * Sets the current clip rect.
 */
void set_clip(const struct rect *r)
{
	if (r == NULL)
		return;

	memcpy(&s_clip, r, sizeof(*r));
}

/*
 * Intersects 'r' with the current clip rect to form the new clip rect.
 */
void add_clip(const struct rect *r)
{
	if (r == NULL)
		return;

	rect_intersect(&s_clip, r, &s_clip);
}

/*
 * Gets the current clip rect.
 */
void get_clip(struct rect *r)
{
	if (r == NULL)
		return;

	memcpy(r, &s_clip, sizeof(*r));
}

static void blit_bmp32_32kc(const unsigned int *src, unsigned int *dst,
			    int rows, int columns,
			    int src_delta, int dst_delta,
		            unsigned int key_color)
{
	int tmp;

	while (rows--) {
		tmp = columns;
		while (tmp--) {
			if (*src != key_color)
				*dst = *src;
			dst++;
			src++;
		}
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32kc_h(const unsigned int *src, unsigned int *dst,
			      int rows, int columns,
			      int src_delta, int dst_delta,
		              unsigned int key_color)
{
	int tmp;

	dst_delta += (columns << 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			dst--;
			if (*src != key_color)
				*dst = *src;
			src++;
		}
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32kc_v(const unsigned int *src, unsigned int *dst,
			      int rows, int columns,
			      int src_delta, int dst_delta,
		              unsigned int key_color)
{
	int tmp;

	dst_delta += columns;
	dst += dst_delta * (rows - 1);
	dst_delta += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			if (*src != key_color)
				*dst = *src;
			src++;
			dst++;
		}
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32kc_hv(const unsigned int *src, unsigned int *dst,
			       int rows, int columns,
			       int src_delta, int dst_delta,
		               unsigned int key_color)
{
	int tmp;

	dst += (dst_delta + columns) * (rows - 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			dst--;
			if (*src != key_color)
				*dst = *src;
			src++;
		}
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32(const unsigned int *src, unsigned int *dst,
			  int rows, int columns,
			  int src_delta, int dst_delta)
{
	dst_delta += columns;
	src_delta += columns;
	columns <<= 2;
	while (rows--) {
		memcpy(dst, src, columns);
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32_h(const unsigned int *src, unsigned int *dst,
			    int rows, int columns,
			    int src_delta, int dst_delta)
{
	int tmp;

	dst_delta += (columns << 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--)
			*--dst = *src++;
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32_v(const unsigned int *src, unsigned int *dst,
			    int rows, int columns,
			    int src_delta, int dst_delta)
{
	dst_delta += columns;
	src_delta += columns;
	dst += dst_delta * (rows - 1);
	columns <<= 2;
	while (rows--) {
		memcpy(dst, src, columns);
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp32_32_hv(const unsigned int *src, unsigned int *dst,
			     int rows, int columns,
			     int src_delta, int dst_delta)
{
	int tmp;

	dst += (dst_delta + columns) * (rows - 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--)
			*--dst = *src++;
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32kc(const unsigned char *src, unsigned int *dst,
			   int rows, int columns,
			   int src_delta, int dst_delta,
			   const unsigned int *src_pal, int src_palsz,
			   unsigned int key_color)
{
	int tmp;
	unsigned int color;

	while (rows--) {
		tmp = columns;
		while (tmp--) {
			if (kassert(*src < src_palsz))
				color = src_pal[*src];
			else
				color = key_color;
			if (color != key_color)
				*dst = color;
			dst++;
			src++;
		}
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32kc_h(const unsigned char *src, unsigned int *dst,
			     int rows, int columns,
			     int src_delta, int dst_delta,
			     const unsigned int *src_pal, int src_palsz,
			     unsigned int key_color)
{
	int tmp;
	unsigned int color;

	dst_delta += (columns << 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			dst--;
			if (kassert(*src < src_palsz))
				color = src_pal[*src];
			else
				color = key_color;
			if (color != key_color)
				*dst = color;
			src++;
		}
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32kc_v(const unsigned char *src, unsigned int *dst,
			     int rows, int columns,
			     int src_delta, int dst_delta,
			     const unsigned int *src_pal, int src_palsz,
			     unsigned int key_color)
{
	int tmp;
	unsigned int color;

	dst_delta += columns;
	dst += dst_delta * (rows - 1);
	dst_delta += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			if (kassert(*src < src_palsz))
				color = src_pal[*src];
			else
				color = key_color;
			if (color != key_color)
				*dst = color;
			src++;
			dst++;
		}
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32kc_hv(const unsigned char *src, unsigned int *dst,
			      int rows, int columns,
			      int src_delta, int dst_delta,
			      const unsigned int *src_pal, int src_palsz,
			      unsigned int key_color)
{
	int tmp;
	unsigned int color;

	dst += (dst_delta + columns) * (rows - 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			dst--;
			if (kassert(*src < src_palsz))
				color = src_pal[*src];
			else
				color = key_color;
			if (color != key_color)
				*dst = color;
			src++;
		}
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32(const unsigned char *src, unsigned int *dst, int rows,
			 int columns, int src_delta, int dst_delta,
			 const unsigned int *src_pal, int src_palsz)
{
	int tmp;

	while (rows--) {
		tmp = columns;
		while (tmp--) {
			if (kassert(*src < src_palsz))
				*dst = src_pal[*src];
			dst++;
			src++;
		}
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32_h(const unsigned char *src, unsigned int *dst,
			   int rows, int columns,
			   int src_delta, int dst_delta,
			   const unsigned int *src_pal, int src_palsz)
{
	int tmp;

	dst_delta += (columns << 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			dst--;
			if (kassert(*src < src_palsz))
				*dst = src_pal[*src];
			src++;
		}
		dst += dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32_v(const unsigned char *src, unsigned int *dst,
			   int rows, int columns,
			   int src_delta, int dst_delta,
			   const unsigned int *src_pal, int src_palsz)
{
	int tmp;

	dst_delta += columns;
	dst += dst_delta * (rows - 1);
	dst_delta += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			if (kassert(*src < src_palsz))
				*dst = src_pal[*src];
			dst++;
			src++;
		}
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp8_32_hv(const unsigned char *src, unsigned int *dst,
			    int rows, int columns,
			    int src_delta, int dst_delta,
			    const unsigned int *src_pal, int src_palsz)
{
	int tmp;

	dst += (dst_delta + columns) * (rows - 1);
	dst += columns;
	while (rows--) {
		tmp = columns;
		while (tmp--) {
			dst--;
			if (kassert(*src < src_palsz))
				*dst = src_pal[*src];
			src++;
		}
		dst -= dst_delta;
		src += src_delta;
	}
}

static void blit_bmp(const unsigned char *src, int src_offs,
		     unsigned char *dst, int dst_offs,
		     int rows, int columns,
		     int src_delta, int dst_delta,
		     const unsigned int *src_pal, int src_palsz,
		     int use_key_color, unsigned int key_color,
		     const unsigned int *dst_pal, int dst_palsz,
		     int transform)
{
	if (kassert_fails(dst_pal == NULL))
		return;

	if (use_key_color) {
		if (src_pal != NULL) {
			switch (transform) {
			default:
			case 0:
				blit_bmp8_32kc(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz, key_color);
				break;
			case FLIPH:
				blit_bmp8_32kc_h(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz, key_color);
				break;
			case FLIPV:
				blit_bmp8_32kc_v(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz, key_color);
				break;
			case FLIPH|FLIPV:
				blit_bmp8_32kc_hv(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz, key_color);
				break;
			}
		} else {
			switch (transform) {
			default:
			case 0:
				blit_bmp32_32kc(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					key_color);
				break;
			case FLIPH:
				blit_bmp32_32kc_h(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					key_color);
				break;
			case FLIPV:
				blit_bmp32_32kc_v(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					key_color);
				break;
			case FLIPH|FLIPV:
				blit_bmp32_32kc_hv(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					key_color);
				break;
			}
		}
	} else {
		if (src_pal != NULL) {
			switch (transform) {
			default:
			case 0:
				blit_bmp8_32(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz);
				break;
			case FLIPH:
				blit_bmp8_32_h(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz);
				break;
			case FLIPV:
				blit_bmp8_32_v(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz);
				break;
			case FLIPH|FLIPV:
				blit_bmp8_32_hv(src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta,
					src_pal, src_palsz);
				break;
			}
		} else {
			switch (transform) {
			default:
			case 0:
				blit_bmp32_32((unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta);
				break;
			case FLIPH:
				blit_bmp32_32_h(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta);
				break;
			case FLIPV:
				blit_bmp32_32_v(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta);
				break;
			case FLIPH|FLIPV:
				blit_bmp32_32_hv(
					(unsigned int *) src + src_offs,
					(unsigned int *) dst + dst_offs,
					rows, columns, src_delta, dst_delta);
				break;
			}
		}
	}
}

/*
 * Draws the rect 'src_rect' of 'bmp' into a 32 bit color 'dst' in position
 * (dx, dy). 
 */
static void draw_bmp32(const struct bmp *src, int dx, int dy,
		       struct bmp *dst, const struct rect *src_rect,
		       int use_key_color, int transform)
{
	int src_rw, dst_rw;
	struct rect p, q, sr;

	/*
	 * Normalize src_rect.
	 */

	p.x = 0;
	p.y = 0;
	p.w = src->w;
	p.h = src->h;
	rect_intersect(src_rect, &p, &sr);

	/*
	 * Current clip.
	 */

	memcpy(&p, &s_clip, sizeof(p));

	/*
	 * Intersect with dest image.
	 */

	q.x = 0;
	q.y = 0;
	q.w = dst->w;
	q.h = dst->h;
	rect_intersect(&p, &q, &p); 
	if (p.w <= 0)
		return;

	/*
	 * Intersect with source rect in dest space.
	 */

	q.x = dx;
	q.y = dy;
	q.w = sr.w;
	q.h = sr.h;
	rect_intersect(&p, &q, &p);
	if (p.w <= 0)
		return;

	q.x = sr.x + p.x - dx;
	q.y = sr.y + p.y - dy;

	if (transform & FLIPH)
		q.x = src_rect->x + src_rect->x + src_rect->w - (q.x + p.w);

	if (transform & FLIPV)
		q.y = src_rect->y + src_rect->y + src_rect->h - (q.y + p.h);

	/* Handle pitch */
	src_rw = src->pitch;
	if (src->pal == NULL)
		src_rw >>= 2;
	dst_rw = dst->pitch;
	if (dst->pal == NULL)
		dst_rw >>= 2;

	/*
	 * Now, from source, we have to take the rect q.x, q.y, p.w, p.h.
	 * In dest, we have to paint in p.x, p.y .
	 */
	blit_bmp(src->pixels, src_rw * q.y + q.x,
		dst->pixels, dst_rw * p.y + p.x,
		p.h, p.w,
		src_rw - p.w, dst_rw - p.w,
		src->pal, src->palsz,
		use_key_color, src->key_color,
		dst->pal, dst->palsz, transform);
}

void draw_bmp_kct(const struct bmp *src, int dx, int dy, struct bmp *dst,
	          const struct rect *src_rect, int use_key_color,
		  int transform)
{
	struct rect r;

	if (kassert_fails(src != NULL))
		return;

	if (kassert_fails(dst != NULL))
		return;

	/*
	 * Drawing to palettized bitmaps is not supported yet.
	 */
	if (kassert_fails(dst->pal == NULL))
		return;

	if (src_rect == NULL) {
		r.x = 0;
		r.y = 0;
		r.w = src->w;
		r.h = src->h;
	} else {
		r = *src_rect;
	}

	draw_bmp32(src, dx, dy, dst, &r, src->use_key_color & use_key_color,
		transform);
}

void draw_bmp(const struct bmp *src, int dx, int dy, struct bmp *dst,
	      const struct rect *src_rect)
{
	draw_bmp_kct(src, dx, dy, dst, src_rect, 1, 0);
}

void set_draw_color(unsigned int color)
{
	s_draw_color = color;
	s_draw_pal_color = color;
}

static void draw_line32(struct bmp *dst, int x, int y,
			int d1x, int d1y, int d2x, int d2y,
		       	int m, int m2, int n)
{
	unsigned int *pixels;
	int p, rw;
       
	pixels = (unsigned int *) dst->pixels;
	rw = dst->pitch >> 2;
	y *= rw;
	d1y *= rw;
	d2y *= rw;
	for (p = 0; p <= m; p++) {
		pixels[y + x] = s_draw_color;
		m2 += n;
		if (m2 >= m) {
			m2 -= m;
			x += d1x;
			y += d1y;
		} else {
			x += d2x;
			y += d2y;
		}
	}
}

static void draw_line8(struct bmp *dst, int x, int y,
		       int d1x, int d1y, int d2x, int d2y,
		       int m, int m2, int n)
{
	int p;

	y *= dst->pitch;
	d1y *= dst->pitch;
	d2y *= dst->pitch;
	for (p = 0; p <= m; p++) {
		dst->pixels[y + x] = s_draw_pal_color;
		m2 += n;
		if (m2 >= m) {
			m2 -= m;
			x += d1x;
			y += d1y;
		} else {
			x += d2x;
			y += d2y;
		}
	}
}

/*
 * Bresenham.
 */
void draw_line(struct bmp *dst, int x0, int y0, int x1, int y1)
{
	int d1x, d1y, d2x, d2y, m, n;
	struct rect r;

	/* clip */
	r.x = 0;
	r.y = 0;
	r.w = dst->w;
	r.h = dst->h;
	rect_intersect(&s_clip, &r, &r);

	if (y1 < y0) {
		m = x0;
		x0 = x1;
		x1 = m;
		m = y0;
		y0 = y1;
		y1 = m;
	}

	if (y0 < r.y)
		y0 = r.y;
	if (y1 >= r.y + r.h)
		y1 = r.y + r.h - 1;
	if (y0 > y1)
		return;

	if (x1 < x0) {
		m = x0;
		x0 = x1;
		x1 = m;
		m = y0;
		y0 = y1;
		y1 = m;
	}

	if (x0 < r.x)
		x0 = r.x;
	if (x1 >= r.x + r.w)
		x1 = r.x + r.w - 1;
	if (x0 > x1)
		return;

	x1 -= x0;
	y1 -= y0;
	d1x = isign(x1);
	d1y = isign(y1);
	d2x = d1x;
	d2y = 0;
	m = iabs(x1);
	n = iabs(y1);
	
	if (m <= n) {
		d2x = 0;
		d2y = d1y;
		m = n;
		n = iabs(x1);
	}
	
	x1 = m >> 1;
	if (dst->pal == NULL) {
		draw_line32(dst, x0, y0, d1x, d1y, d2x, d2y, m, x1, n);
	} else {
		draw_line8(dst, x0, y0, d1x, d1y, d2x, d2y, m, x1, n);
	}
}

void bmp_draw_init(void)
{
	s_draw_color = 0;
	s_draw_pal_color = 0;
	reset_clip(0, 0);
}
