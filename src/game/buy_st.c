/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "buy_st.h"
#include "game.h"
#include "menu.h"
#include "data.h"
#include "sounds.h"
#include "msgbox.h"
#include "menu_st.h"
#include "tilengin.h"
#include "fade.h"
#include "fade_st.h"
#include "initfile.h"
#include "modplay.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/cbase.h"
#include "cfg/cfg.h"

#ifndef STDIO_H
#include <stdio.h>
#endif

static char s_url[128];

#define TX_BUY "YES"
#define TX_NOT_NOW "NOT NOW"

enum {
	OP_BUY,
	OP_NOT_NOW,
};

static struct menu s_buymenu = {
	.options = {
		{ TX_BUY, OP_BUY },
		{ TX_NOT_NOW, OP_NOT_NOW },
		{ NULL, -2 },
	},
};

static void update_buymenu(void)
{
	const struct kernel_device *d;
	int r;

	d = kernel_get_device();
	r = menu_update();
	switch (r) {
	case OP_BUY:
		modplay_stop();
		mixer_play(wav_opsel);
		d->open_url(s_url);
		break;
	case OP_NOT_NOW:
	       	mixer_play(wav_opsel);
		end_state();
		break;
	case -2: 
		mixer_play(wav_opmove);
		break;
	}
}

static void update_menu_actor(struct actor *pac)
{
	update_buymenu();
}

static struct actor *spawn_buymenu_fp(int x, int y)
{
	struct actor *pac;
	int r;

	/* url to buy */
	r = tokscanf(NULL, "S", s_url, NELEMS(s_url));
	if (r != 1) {
		s_url[0] = '\0';
	}

	pac = get_actor(AC_MENU);
	pac->update = update_menu_actor;
	menu_reset_stack();
	menu_push(&s_buymenu, y, -1, NULL);
	return pac;
}

static void draw(void)
{
	te_begin_draw();
	te_draw();
	fade_draw();
	te_end_draw();
}

static void update(void)
{
	exec_update_fns();
	update_actors();
	exec_init_map_code();
}

static void enter(const struct state *old_state)
{
	load_map(initfile_getvar("buy_map"));
}

static void end(void)
{
	fade_to_state(&menu_st);
}

const struct state buy_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end
};

void buy_st_init(void)
{
	if (PP_DEMO) {
		register_spawn_fn("buymenu", spawn_buymenu_fp);
	}
}
