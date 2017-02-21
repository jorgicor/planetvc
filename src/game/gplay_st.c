/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "gplay_st.h"
#include "game.h"
#include "input.h"
#include "tilengin.h"
#include "oxigen.h"
#include "stargate.h"
#include "strdraw.h"
#include "hud.h"
#include "menu_st.h"
#include "over_st.h"
#include "end_st.h"
#include "win_st.h"
#include "fade_st.h"
#include "msgbox.h"
#include "cheats.h"
#include "cosmonau.h"
#include "data.h"
#include "modplay.h"
#include "initfile.h"
#include "cheats.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"

static void update_playing(void);
static void update_exitq(void);

static void (*s_update_fn)(void);

enum {
	OP_EXIT_YES,
	OP_EXIT_NO
};

static const struct msgbox s_exit_msgbox = {
	.title = "EXIT TO MENU?",
	.options = { 
		{ "YES", OP_EXIT_YES },
		{ "NO" , OP_EXIT_NO},
	},
	.x = 5,
       	.w = TE_FMW - 10,
       	.y = (TE_FMH - 5) / 2,
	.can_go_back = 1
};

static void draw(void)
{
	te_begin_draw();
	te_draw();
	te_end_draw();
}

static void update_exitq(void)
{
	int r;

	r = update_msgbox();
	if (r == OP_EXIT_YES) {
		mixer_play(wav_opsel);
		kernel_get_device()->clear_down_keys();
		end_state();
	} else if (r == OP_EXIT_NO || r == MSGBOX_BACK) {
		mixer_play(wav_opsel);
		te_enable_anims(1);
		kernel_get_device()->clear_down_keys();
		s_update_fn = update_playing;
	}
}

static void set_exitq(void)
{
	show_msgbox(&s_exit_msgbox);
	te_enable_anims(0);
	s_update_fn = update_exitq;
}

static void update_playing(void)
{
	const struct kernel_device *d;

	d = kernel_get_device();
	if (is_first_pressed(LKEYY)) {
		mixer_play(wav_opmove);
		set_exitq();
		return;
	}

	if (s_cheats_debug_mode) {
		if (d->key_first_pressed(KERNEL_KSC_1))
			restart_map(0);
		else if (d->key_first_pressed(KERNEL_KSC_2))
			restart_map(1);
		else if (d->key_first_pressed(KERNEL_KSC_3))
			restart_map(2);
		else if (d->key_first_pressed(KERNEL_KSC_4))
			restart_map(3);
		else if (d->key_first_pressed(KERNEL_KSC_L))
			restart_map(-2);
		else if (d->key_first_pressed(KERNEL_KSC_O))
			s_got_oxigen = 1;
		else if (d->key_first_pressed(KERNEL_KSC_D))
			te_draw_bboxes ^= 1;
	}

	if (is_map_exit_scheduled())
		exit_map();
	else if (is_teleport_scheduled())
		teleport();
	else if (is_restart_scheduled())
		restart_map(-1);

	exec_update_fns();
	update_actors();
	exec_init_map_code();

	if (g_state == &gplay_st) {
		if (is_gameover_scheduled()) {
			fade_to_state(&over_st);
		} else if (is_win_scheduled()) {
			fade_to_state(&end_st);
#if 0
			if (initfile_getvar("demo_version")) {
				fade_to_state(&win_st);
			} else {
				fade_to_state(&end_st);
			}
#endif
		}
	}
}

static void update(void)
{
	kasserta(s_update_fn != NULL);
	s_update_fn();
}

static void enter(const struct state *old_state)
{
	hud_game_started();
	reset_exit_side_backup();

	s_active_stargate_id = initfile_getvar("active_portal");
	if (s_cheats_translator_mode) {
		s_active_stargate_id = initfile_getvar("translators_portal");
	}

	load_map(initfile_getvar("start_map"));
	cosmonaut_start_at_gate(initfile_getvar("start_portal"));

	s_update_fn = update_playing;
}

static void leave(const struct state *new_state)
{
	modplay_stop();
}

static void end(void)
{
	fade_to_state(&menu_st);
}

const struct state gplay_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end,
	.leave = leave,
};
