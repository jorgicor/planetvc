/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "demo_st.h"
#include "tilengin.h"
#include "game.h"
#include "input.h"
#include "cosmonau.h"
#include "modplay.h"
#include "menu_st.h"
#include "msgbox.h"
#include "fade_st.h"
#include "text.h"
#include "strdraw.h"
#include "initfile.h"
#include "gamelib/vfs.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

enum {
	NFRAME_KEYS = 256,
	RANDOM = 1,
	MAX_DEMOS = 256,
};

static struct frame_keys {
	int frame;
	int keys;
} s_frame_keys[NFRAME_KEYS];

static int s_frames;
static int s_max_frames;
static int s_ndemos;
static int s_last_index;	/* last demo index loaded */

/* Contains the mixed demo indexes. */
static int s_demo_list[MAX_DEMOS];

static void load_random_demo(void);

/* s must be untranslated. */
static void draw_centered(const char *s, int y)
{
	int x;

	s = _(s);
	x = (TE_FMW - utf8_strlen(s)) / 2;
	draw_str(s, x, y, 0);
}

static void draw_hint(void)
{
	draw_centered("'ENTER' TO SKIP", TE_FMH - 2);
	draw_centered("'ESC' TO EXIT", TE_FMH - 1);
}

static void draw(void)
{
	te_begin_draw();
	te_draw();
	te_end_draw();
}

int demo_is_key_down(int key)
{
	int i, k, keys, frame;

	k = 0;
	for (i = KUP; i < KEYB; i++) {
	       if (key == i) {
		       k = 1 << i; 
		       break;
	       }
	}	       

	keys = 0;
	for (i = 0; i < NFRAME_KEYS; i++) {
		frame = s_frame_keys[i].frame;
		if (frame == -1)
			return 0;
		else if (frame == s_frames)
			return (s_frame_keys[i].keys & k) != 0;
		else if (frame < s_frames)
			keys = s_frame_keys[i].keys;
		else if (frame > s_frames)
			return (keys & k) != 0;
	}

	return 0;
}

static void update(void)
{
	if (s_ndemos == 0) {
		end_state();
		return;
	}

	if (is_first_pressed(LKEYY)) {
		mixer_play(wav_opmove);
		end_state();
		return;
	}

	exec_update_fns();
	update_actors();
	exec_init_map_code();

	s_frames++;
	if (is_first_pressed(LKEYA) || s_frames >= s_max_frames) {
		load_random_demo();
		draw_hint();
	}
}

static void load_random_demo(void)
{
	FILE *fp;
	int mapid, x, y, dir, ax, vx, ay, vy;
	int jumping, walking;
	int r, frame, keys, i;
	char fname[32];

	if (s_ndemos == 0)
		return;

	s_frames = 0;
	s_max_frames = 1;
	s_last_index = (s_last_index + 1) % s_ndemos;
	i = s_demo_list[s_last_index];

	snprintf(fname, sizeof(fname), "data/demo_%d.txt", i);
	fp = open_file(fname, NULL);
	if (fp == NULL) {
		s_ndemos = 0;
		return;
	}

	fscanf(fp, " map %d ", &mapid);

	/* ktrace("loading map %d %x", i, mapid); */

	load_map(mapid);

	fscanf(fp, " x %d y %d dir %d ", &x, &y, &dir);
	fscanf(fp, " jump %d walk %d ", &jumping, &walking);
	fscanf(fp, " ax %d vx %d ay %d vy %d ", &ax, &vx, &ay, &vy);
	i = 0;
	if (jumping)
		i = 1;
	else if (walking)
		i = 2;
	cosmonaut_set_for_demo(x, y, dir, i, ax, vx, ay, vy);

	i = 0;
	while (i < NFRAME_KEYS - 1 &&
		(r = fscanf(fp, " %d %d ", &frame, &keys)) == 2)
       	{
		s_frame_keys[i].frame = frame;
		s_frame_keys[i].keys = keys;
		i++;
	}
	s_frame_keys[i].frame = -1;
	if (i > 0) {
		s_max_frames = s_frame_keys[i - 1].frame + FPS * 2;
	}

	fclose(fp);
}

static void mix_demos(void)
{
	int i, n, k;

	n = s_ndemos;
	while (n > 1) {
		i = rand() % n;
		k = s_demo_list[i];
		s_demo_list[i] = s_demo_list[n - 1];
		s_demo_list[n - 1] = k;
		n--;
	}
}

static void fill_demo_list(void)
{
	int i;

	for (i = 0; i < s_ndemos; i++)
		s_demo_list[i] = i;
}

static void load_demo_list(void)
{
	s_ndemos = initfile_getvar("ndemos");
	if (s_ndemos < 0)
		s_ndemos = 0;
	if (s_ndemos > MAX_DEMOS)
		s_ndemos = MAX_DEMOS;

	fill_demo_list();
	s_last_index = (s_ndemos > 1) ? s_ndemos - 1 : 0;
}

static void enter(const struct state *old_state)
{
	modplay_free();
	load_demo_list();
	if (RANDOM) {
		mix_demos();
	}
	load_random_demo();
	draw_hint();
}

static void end(void)
{
	fade_to_state(&menu_st);
}

const struct state demo_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end,
};
