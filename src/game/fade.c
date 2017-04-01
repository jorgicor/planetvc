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

#include "fade.h"
#include "tilengin.h"
#include "game.h"

enum {
	FADE_DELAY = 4,
	TIMES = 9,
};

static int s_time;
static int s_times;
static int s_enabled;
static int s_fadein;

/* 
 * This should be called fater te_begin() and te_draw(),
 * and before te_end().
 */
void fade_draw(void)
{
	unsigned int *pixels;
	unsigned int c, p;
	int x, y, rgbsel, t;

	if (!s_enabled)
		return;

	rgbsel = 0;
	t = s_times;
	if (t > TIMES)
		t = TIMES;
	if (s_fadein)
		t = TIMES - t;
loop:
	if (t <= 0)
		return;
	for (y = 0; y < te_screen.h; y++) {
		pixels = (unsigned int *) (te_screen.pixels +
		       	te_screen.pitch * y);
		for (x = 0; x < te_screen.w; x++) {
			p = pixels[x];
			switch (rgbsel) {
			case 0: c = p & 0xff; break;
			case 1: c = (p >> 8) & 0xff; break;
			case 2: c = (p >> 16) & 0xff; break;
			}

			if (c >= 255) {
				c = 128;
			} else {
				c = 0;
			}

			switch (rgbsel) {
			case 0: p = (p & ~0xff) | c; break;
			case 1: p = (p & ~0xff00) | (c << 8); break;
			case 2: p = (p & ~0xff0000) | (c << 16); break;
			}

			pixels[x] = p;
		}
	}
	t--;
	rgbsel = (rgbsel + 1) % 3;
	goto loop;
}

int is_fade_over(void)
{
	return s_times > 9;
}

void fade_update(void)
{
	if (is_fade_over())
		return;
	s_time = dectime(s_time);
	if (s_time > 0)
		return;
	s_time = FADE_DELAY;
	s_times++;
}

void fade_in(void)
{
	s_times = 0;
	s_time = FADE_DELAY;
	s_enabled = 1;
	s_fadein = 1;
}

void fade_out(void)
{
	s_times = 0;
	s_time = FADE_DELAY;
	s_enabled = 1;
	s_fadein = 0;
}

void disable_fade()
{
	s_enabled = 0;
}
