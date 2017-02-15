/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "pad.h"
#include "tilengin.h"
#include "bitmaps.h"
#include "gamelib/bmp.h"
#include "kernel/kernel.h"
#include "cbase/cbase.h"
#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif

enum {
#if defined(PP_ANDROID_MODE)
	PADH = 7,
#else
	PADH = 0,
#endif
	PAD_SCRH = PADH * TE_BTW,
	BDELTA = 12,
	DX = TE_SCRW / 8,
	DY = PAD_SCRH / 4,
};

static struct bmp *bmp_pad = NULL;

/* The portion where we draw */
static struct bmp s_screen; 

static struct xypos {
	int x;
	int y;
} s_button_pos[] = {
	{ 2 * DX - BDELTA, DY - BDELTA },
	{ 2 * DX - BDELTA, 3 * DY - BDELTA },
	{ DX - BDELTA, 2 * DY - BDELTA },
	{ 3 * DX - BDELTA, 2 * DY - BDELTA },

	{ 6 * DX - BDELTA, DY - BDELTA },
	{ 6 * DX - BDELTA, 3 * DY - BDELTA },
	{ 5 * DX - BDELTA, 2 * DY - BDELTA },
	{ 7 * DX - BDELTA, 2 * DY - BDELTA },
};

enum {
	NBUTTONS = NELEMS(s_button_pos),
};

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
		draw_bmp(bmp_pad, s_button_pos[i].x, s_button_pos[i].y,
			 &s_screen, NULL);
	}
}

void update_pad(void)
{
	if (PADH > 0 && bmp_pad != NULL) {
		draw_pad();
	}
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
