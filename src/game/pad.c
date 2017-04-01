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

#include "pad.h"
#include "tilengin.h"
#include "bitmaps.h"
#include "game.h"
#include "gamelib/bmp.h"
#include "kernel/kernel.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <string.h>

enum {
#if PP_PHONE_MODE
	PADH = 7,
#else
	PADH = 0,
#endif
	BUTTONS_PER_ROW = 4,
	PAD_SCRH = PADH * TE_BTW,
	DX = TE_SCRW / 4,
	DY = PAD_SCRH / 2,
};

static struct bmp *bmp_pad = NULL;

/* The portion where we draw */
static struct bmp s_screen; 

static const struct xypos {
	int x;
	int y;
	int ksc;
} s_button_pos[] = {
	{ 0, 0, KERNEL_KSC_PAD0_X },
	{ DX, 0, KERNEL_KSC_PAD0_Y },
	{ 2 * DX, 0, KERNEL_KSC_PAD0_B },
	{ 3 * DX, 0, KERNEL_KSC_PAD0_A },
	
	{ 0, DY, KERNEL_KSC_PAD0_DLEFT },
	{ DX, DY, KERNEL_KSC_PAD0_DRIGHT },
	{ 2 * DX, DY, KERNEL_KSC_PAD0_DUP },
	{ 3 * DX, DY, KERNEL_KSC_PAD0_DDOWN },
};

enum {
	NBUTTONS = NELEMS(s_button_pos),
};

static signed char s_button_was_down[NBUTTONS];
static signed char s_button_down[NBUTTONS];
static unsigned char s_button_draw_count[NBUTTONS];

static void draw_pad(void)
{
	int i;
	const struct kernel_device *d;
	struct kernel_canvas *kcanvas;
	struct rect r;

	d = kernel_get_device();
	kcanvas = d->get_canvas();

	s_screen.pixels = ((unsigned char *) kcanvas->pixels) +
	       	kcanvas->pitch * TE_SCRH;
	s_screen.w = kcanvas->w;
	s_screen.h = PAD_SCRH;
	s_screen.pitch = kcanvas->pitch;

	reset_clip(s_screen.w, s_screen.h);

	r.w = DX;
	r.h = DY;
	for (i = 0; i < NBUTTONS; i++) {
		if (s_button_draw_count[i] == 0) {
			r.x = s_button_pos[i].x; 
			r.y = s_button_pos[i].y;
			if (s_button_down[i]) {
				r.y += DY * 2;
			}
			draw_bmp(bmp_pad, s_button_pos[i].x, s_button_pos[i].y,
				 &s_screen, &r);
		}
		/* This will make the button draw inconditionally if a 
		 * second passes...
		 * As we are only drawing buttons when changed, I am not sure
		 * if the surface where we draw can go away... We must suppose
		 * that maybe it can go away and be recreated, thus having
		 * garbage...
		 */
		s_button_draw_count[i] = (s_button_draw_count[i] + 1) % FPS;
	}
}

void update_pad(void)
{
	const struct kernel_device *d;
	const struct kernel_finger *kfinger;
	float scale, dy;
	int i, b, x, y, scrw, scrh;

	if (!(PADH > 0 && bmp_pad != NULL)) {
		return;
	}

	d = kernel_get_device();
	d->get_window_size(&scrw, &scrh);
	scale = (scrw * 1.f) / TE_SCRW;
	dy = (scrh + scale * (TE_SCRH - PAD_SCRH)) / 2;

	// ktrace("winsize %d %d scale %g dy %d", scrw, scrh, scale, dy);

	memcpy(s_button_was_down, s_button_down, sizeof(s_button_down));
	memset(s_button_down, 0, sizeof(s_button_down));
	for (i = 0; i < KERNEL_NFINGERS; i++) {
		kfinger = d->get_finger(i);
		if (kfinger == NULL) {
			continue;
		}
		x = (kfinger->x * scrw) / scale;
		y = ((kfinger->y * scrh) - dy) / scale;
		// ktrace("finger %g %g %d %d", kfinger->x, kfinger->y, x, y);
		for (b = 0; b < NBUTTONS; b++) {
			if (x >= s_button_pos[b].x &&
			    x < s_button_pos[b].x + DX &&
			    y >= s_button_pos[b].y &&
			    y < s_button_pos[b].y + DY)
		       	{
				if (s_button_was_down[b]) {
					s_button_down[b] = 1;
				} else if(!s_button_down[b]) {
					s_button_draw_count[b] = 0;
					s_button_down[b] = 1;
					d->insert_pad_event(1,
						s_button_pos[b].ksc);
				}
			}
		}
#if 0
		for (b = 0; b < NBUTTONS; b++) {
			distx = iabs(x - s_button_pos[b].x);
			disty = iabs(y - s_button_pos[b].y);
			if (distx * distx + disty * disty <= RADIUS2) {
				if (s_button_was_down[b]) {
					s_button_down[b] = 1;
				} else if(!s_button_down[b]) {
					s_button_down[b] = 1;
					d->insert_pad_event(1,
						s_button_pos[b].ksc);
				}
			}
		}
#endif
	}

	for (b = 0; b < NBUTTONS; b++) {
		if (s_button_was_down[b] && !s_button_down[b]) { 
			s_button_draw_count[b] = 0;
			d->insert_pad_event(0, s_button_pos[b].ksc);
		}
	}

	draw_pad();
}

int pad_scrh(void)
{
	return PAD_SCRH;
}

void pad_init(void)
{
	memset(s_button_was_down, 0, sizeof(s_button_was_down));
	memset(s_button_down, 0, sizeof(s_button_down));
	memset(s_button_draw_count, 0, sizeof(s_button_draw_count));
	if (PADH > 0) {
		register_bitmap(&bmp_pad, "pad", 0, 0);
	}
}
