/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "end_st.h"
#include "tilengin.h"
#include "fade.h"
#include "fade_st.h"
#include "win_st.h"
#include "game.h"
#include "strdraw.h"
#include "initfile.h"
#include "google.h"
#include "cfg/cfg.h"

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
	static const char *last_achiev[NDIFFICULTY_LEVELS] = {
		"CgkIk_7jut4bEAIQEw",
		"CgkIk_7jut4bEAIQDQ"
	};

	if (!PP_DEMO && Android_IsConnectedToGooglePlay()) {
		Android_UnlockAchievement(last_achiev[get_difficulty()]);
	}

	load_map(initfile_getvar("end_map"));
}

static void end(void)
{
	fade_to_state(&win_st);
}

const struct state end_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end
};
