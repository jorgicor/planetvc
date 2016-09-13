/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "hud.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "gplay_st.h"
#include "end_st.h"
#include "text.h"
#include "strdraw.h"
#include "cosmonau.h"
#include "input.h"
#include "cheats.h"
#include "msgbox.h"
#include "initfile.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

static struct bmp *bmp_hud;

static FRAME(hud_cosmo, bmp_hud, 0, 0, 16, 16);
static FRAME(hud_x, bmp_hud, 16, 0, 8, 16);
static FRAME(hud_dig0, bmp_hud, 24, 0, 8, 16);
static FRAME(hud_dig1, bmp_hud, 32, 0, 8, 16);
static FRAME(hud_dig2, bmp_hud, 40, 0, 8, 16);
static FRAME(hud_dig3, bmp_hud, 48, 0, 8, 16);
static FRAME(hud_dig4, bmp_hud, 56, 0, 8, 16);
static FRAME(hud_dig5, bmp_hud, 64, 0, 8, 16);
static FRAME(hud_dig6, bmp_hud, 72, 0, 8, 16);
static FRAME(hud_dig7, bmp_hud, 80, 0, 8, 16);
static FRAME(hud_dig8, bmp_hud, 88, 0, 8, 16);
static FRAME(hud_dig9, bmp_hud, 96, 0, 8, 16);

static struct frame *digit_frames[] = {
	&fr_hud_dig0,
	&fr_hud_dig1,
	&fr_hud_dig2,
	&fr_hud_dig3,
	&fr_hud_dig4,
	&fr_hud_dig5,
	&fr_hud_dig6,
	&fr_hud_dig7,
	&fr_hud_dig8,
	&fr_hud_dig9,
};

static const char *tx_restart[] = {
	"PRESS 'RESTART'",
	"OR 'ENTER'"
};

static const char *tx_training[] = {
	"PRACTICE",
	"MODE"
};

enum {
	HUD_Y = (TE_BMH - 1) * TE_VBTW,
	RESTART_DELAY = FPS_FULL,
	HUD_RESTART_Y = TE_FMH - NELEMS(tx_restart),
	HUD_TRAINING_Y = TE_FMH - NELEMS(tx_training),
	OVER_DELAY = FPS_FULL * 3,
};

#define SHOWING	i0

/* Number of lifes + the one playing.
 * It is immediately decremented when the player is touched (dying).
 */
static int s_total_nlifes;

/* Number of lifes remaining.
 * This is what we show in the HUD.
 */
static int s_nlifes;
static int s_last_nlifes;

/* If we toggled training mode on or off, a restart is scheduled. */
static int s_restart_scheduled;

/* If we are in training mode. */
static int s_training;

/* time to game over screen passed. */
static int s_gameover_scheduled;

/* When we die and have no more lifes we start
 * passing time until showing the game over screen.
 */
static int s_over_t;
static int s_over_countdown_active;

/* Each bit marks a visited map.
 * It is reset each time we cross a portal.
 */
static unsigned short s_visited[16];

/* Rooms visited to calculate progression. */
static int s_nvisited;

int is_training(void)
{
	return s_training;
}

static void reset_training_mode(void)
{
	s_training = 0;
}

static void reset_nlifes(void)
{
	s_total_nlifes = 10;	
	s_nlifes = s_total_nlifes;
}

static void reset_visited_map(void)
{
	memset(s_visited, 0, sizeof(s_visited));
}

static void visit_map(int id)
{
	unsigned short n, i, b;

	if (kassert_fails(id >= 0 && id <= 255))
		return;

	i = id / 16;
	n = s_visited[i];
	b = (unsigned) 1 << (id % 16);
	if ((n & b) != 0)
		return;
	s_visited[i] |= b; 
	s_nvisited++;
}

void hud_game_started(void)
{
	reset_nlifes();
	reset_training_mode();
	reset_visited_map();
	s_nvisited = 0;
}

void hud_teleported(void)
{
	reset_visited_map();
}

int get_nvisited_maps(void)
{
	return s_nvisited;
}

static void show_nvisited(void)
{
	int x;
	char str[16];

	if (s_training)
		return;

	snprintf(str, sizeof(str), "%d/%d", s_nvisited, 
		 initfile_getvar("nmaps_to_win"));
	x = TE_FMW - strlen(str);
	draw_str(str, x, TE_FMH - 1, 0);
}

static void draw_nlifes(void)
{
	struct sprite *psp;
	int n;

	n = s_nlifes;
	if (kassert_fails(s_nlifes >= 0 && s_nlifes <= 99))
			n = 0;

	kasserta((n / 10) < NELEMS(digit_frames));
	kasserta((n % 10) < NELEMS(digit_frames));

	if (s_nlifes <= 9) {
		te_free_sprite(te_get_sprite(SP_HUD_DIG1));
		psp = te_get_sprite(SP_HUD_DIG0);
		psp->pframe = digit_frames[n];
		psp->x = TE_VFTW * 3;
		psp->y = HUD_Y;
		psp->flags = SP_F_TOP;
	} else {
		psp = te_get_sprite(SP_HUD_DIG0);
		psp->pframe = digit_frames[n / 10];
		psp->x = TE_VFTW * 3;
		psp->y = HUD_Y;
		psp->flags = SP_F_TOP;

		psp = te_get_sprite(SP_HUD_DIG1);
		psp->pframe = digit_frames[n % 10];
		psp->x = TE_VFTW * 4;
		psp->y = HUD_Y;
		psp->flags = SP_F_TOP;
	}
}

static void clear_restart(void)
{
	int txi, i, x, w;

	for (txi = 0; txi < NELEMS(tx_restart); txi++) {
		w = utf8_strlen(_(tx_restart[txi]));
		x = (TE_FMW - w) / 2;
		for (i = 0; i < w; i++)
			draw_str(" ", x + i, HUD_RESTART_Y + txi, 0);
	}
}

static void hud_restart_blink(struct actor *pac)
{
	int x, txi;

	pac->t = dectime(pac->t);
	if (pac->t > 0)
		return;

	pac->t = RESTART_DELAY;
	if (pac->SHOWING) {
		pac->SHOWING = 0;
		clear_restart();
	} else {
		pac->SHOWING = 1;
		for (txi = 0; txi < NELEMS(tx_restart); txi++) {
			x = (TE_FMW - utf8_strlen(_(tx_restart[txi]))) / 2;
			draw_str(_(tx_restart[txi]), x, HUD_RESTART_Y + txi,
				0);
		}
	}
}

static void spawn_hud_restart(void)
{
	struct actor *pac;

	pac = get_actor(AC_HUD_RESTART);
	pac->t = 0;
	pac->SHOWING = 0;
	pac->update = hud_restart_blink;
}

static void update_over_countdown(void)
{
	if (!s_over_countdown_active)
		return;

	s_over_t = dectime(s_over_t);
	if (s_over_t > 0)
		return;

	s_over_countdown_active = 0;
	s_gameover_scheduled = 1;
}

static void start_over_countdown(void)
{
	s_over_countdown_active = 1;
	s_over_t = OVER_DELAY;
}

int is_gameover_scheduled(void)
{
	return s_gameover_scheduled;
}

static void hud_update(struct actor *pac)
{
	const struct kernel_device *d;

	d = kernel_get_device();
	if (s_cheats_debug_mode && s_total_nlifes > 0 &&
		!cosmonaut_is_dead() &&
		d->key_first_pressed(KERNEL_KSC_LCTRL))
	{
		cosmonaut_set_dying(1);
		s_total_nlifes = 1;
	}

	if (s_nlifes != s_last_nlifes) {
		s_last_nlifes = s_nlifes;
		draw_nlifes();
	}

	update_over_countdown();

	if (s_total_nlifes > 0 && cosmonaut_is_dead()
		&& is_free_actor(get_actor(AC_HUD_RESTART)))
	{
		if (!s_training)
			s_total_nlifes--;
		if (s_total_nlifes > 0)
			spawn_hud_restart();
		else
			start_over_countdown();
	}

	if (s_total_nlifes > 0 && !cosmonaut_is_teleporting()) {
		if (cosmonaut_is_dead() && is_first_pressed(LKEYA)) {
			/* To allow for the enter key when dead. */
			mixer_play(wav_opmove);
			s_restart_scheduled = 1;
		} else if (is_first_pressed(KEYB)) {
			mixer_play(wav_opmove);
			s_restart_scheduled = 1;
		} else if (is_first_pressed(KEYX)) {
			mixer_play(wav_opmove);
			s_training = !s_training;
			s_restart_scheduled = 1;
		}
	}
}

#if 0
static void hide_hud(void)
{
	te_free_sprite(te_get_sprite(SP_HUD_COSMO));
	te_free_sprite(te_get_sprite(SP_HUD_X));
	te_free_sprite(te_get_sprite(SP_HUD_DIG0));
	te_free_sprite(te_get_sprite(SP_HUD_DIG1));
	free_actor(get_actor(AC_HUD));
}
#endif

static void show_hud(void)
{
	struct actor *pac;
	struct sprite *psp;

	pac = get_actor(AC_HUD);
	if (!is_free_actor(pac))
		return;

	pac->update = hud_update;

	psp = te_get_sprite(SP_HUD_COSMO);
	psp->pframe = &fr_hud_cosmo;
	psp->x = 0;  
	psp->y = HUD_Y;
	psp->flags = SP_F_TOP;

	psp = te_get_sprite(SP_HUD_X);
	psp->pframe = &fr_hud_x;
	psp->x = TE_VBTW;  
	psp->y = HUD_Y;
	psp->flags = SP_F_TOP;

	draw_nlifes();
}

static void show_training(void)
{
	int i, w, maxw;

	if (!s_training)
		return;

	maxw = 0;
	for (i = 0; i < NELEMS(tx_training); i++) {
		w = utf8_strlen(_(tx_training[i]));
		if (w > maxw)
			maxw = w;
	}

	for (i = 0; i < NELEMS(tx_training); i++) {
		w = utf8_strlen(_(tx_training[i]));
		draw_str(_(tx_training[i]),
			TE_FMW - maxw + (maxw - w) / 2,
			HUD_TRAINING_Y + i, 0);
	}
}

static void clear_bottom(void)
{
	te_fill_fg(0, TE_FMH - 2, TE_FMW, 2, 0, chri(' '));
}

int is_restart_scheduled(void)
{
	return s_restart_scheduled;
}

static void spawn_hud(int mapid)
{
	if (g_state == &gplay_st) {
		s_nlifes = s_total_nlifes - 1;
		s_restart_scheduled = 0;
		s_gameover_scheduled = 0;
		s_over_countdown_active = 0;
		visit_map(mapid);
		show_hud();
		clear_bottom();
		show_training();
		show_nvisited();
	}
}

void hud_init(void)
{
	register_bitmap(&bmp_hud, "hud", 1, 0x8080);
	register_map_init_fn(spawn_hud);
}
