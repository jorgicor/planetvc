/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "pad.h"
#include "tilengin.h"
#include "bitmaps.h"
#include "gamelib/bmp.h"
#include "kernel/kernel.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif
#include <string.h>

enum {
#if defined(PP_PHONE_MODE)
	PADH = 7,
#else
	PADH = 0,
#endif
	PAD_SCRH = PADH * TE_BTW,
	BDELTA = 12,
	DX = TE_SCRW / 8,
	DY = PAD_SCRH / 4,
	BUTTON_W = 25,
	BUTTON_H = 25,
	RADIUS2 = 20 * 20,
};

static struct bmp *bmp_pad = NULL;

/* The portion where we draw */
static struct bmp s_screen; 

static struct xypos {
	int x;
	int y;
	int ksc;
} s_button_pos[] = {
	{ DX, DY, KERNEL_KSC_PAD0_Y },
	{ 3 * DX, DY, KERNEL_KSC_PAD0_X },
	{ 5 * DX, DY, KERNEL_KSC_PAD0_DUP },
	{ 7 * DX, DY, KERNEL_KSC_PAD0_A },

	{ DX, 3 * DY, KERNEL_KSC_PAD0_DLEFT },
	{ 3 * DX, 3 * DY, KERNEL_KSC_PAD0_DRIGHT },
	{ 5 * DX, 3 * DY, KERNEL_KSC_PAD0_B },
	{ 7 * DX, 3 * DY, KERNEL_KSC_PAD0_DDOWN },

#if 0
	{ 2 * DX, DY, KERNEL_KSC_PAD0_DUP },
	{ 2 * DX, 3 * DY, KERNEL_KSC_PAD0_DDOWN },
	{ DX, 2 * DY, KERNEL_KSC_PAD0_DLEFT },
	{ 3 * DX, 2 * DY, KERNEL_KSC_PAD0_DRIGHT },

	{ 6 * DX, DY, KERNEL_KSC_PAD0_Y },
	{ 6 * DX, 3 * DY, KERNEL_KSC_PAD0_A },
	{ 5 * DX, 2 * DY, KERNEL_KSC_PAD0_X },
	{ 7 * DX, 2 * DY, KERNEL_KSC_PAD0_B },
#endif
};

enum {
	NBUTTONS = NELEMS(s_button_pos),
};

static signed char s_button_was_down[NBUTTONS];
static signed char s_button_down[NBUTTONS];

static void draw_pad(void)
{
	int i;
	const struct kernel_device *d;
	struct kernel_canvas *kcanvas;

	d = kernel_get_device();
	kcanvas = d->get_canvas();

	s_screen.pixels = ((unsigned char *) kcanvas->pixels) +
	       	kcanvas->pitch * TE_SCRH;
	s_screen.w = kcanvas->w;
	s_screen.h = PAD_SCRH;
	s_screen.pitch = kcanvas->pitch;

	reset_clip(s_screen.w, s_screen.h);

	for (i = 0; i < NBUTTONS; i++) {
		draw_bmp(bmp_pad, s_button_pos[i].x - BDELTA,
			 s_button_pos[i].y - BDELTA, &s_screen, NULL);
	}
}

void update_pad(void)
{
	const struct kernel_device *d;
	const struct kernel_finger *kfinger;
	float scale, dy;
	int i, b, x, y, scrw, scrh, distx, disty;

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
			if (x > s_button_pos[b].x - DX &&
			    x < s_button_pos[b].x + DX &&
			    y > s_button_pos[b].y - DY &&
			    y < s_button_pos[b].y + DY)
		       	{
				if (s_button_was_down[b]) {
					s_button_down[b] = 1;
				} else if(!s_button_down[b]) {
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
	if (PADH > 0) {
		register_bitmap(&bmp_pad, "pad", 0, 0);
	}
}
