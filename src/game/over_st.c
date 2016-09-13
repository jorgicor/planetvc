/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "over_st.h"
#include "game.h"
#include "tilengin.h"
#include "fade_st.h"
#include "menu_st.h"
#include "cheats.h"
#include "modplay.h"
#include "initfile.h"
#include "kernel/kernel.h"

static void draw(void)
{
	te_begin_draw();
	te_draw();
	te_end_draw();
}

static void update(void)
{
	const struct kernel_device *d;

	d = kernel_get_device();
	if (s_cheats_debug_mode && d->key_first_pressed(KERNEL_KSC_R)) {
		load_map(initfile_getvar("over_map"));
	}

	exec_update_fns();
	update_actors();
	exec_init_map_code();
}

static void enter(const struct state *old_state)
{
	load_map(initfile_getvar("over_map"));
}

static void end(void)
{
	fade_to_state(&menu_st);
}

static void leave(const struct state *new_state)
{
	modplay_stop();
}

const struct state over_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end,
	.leave = leave,
};
