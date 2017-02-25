/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "game.h"
#include "load_st.h"
#include "tilengin.h"
#include "bitmaps.h"
#include "sounds.h"
#include "menu_st.h"
#include "data.h"
#include "text.h"
#include "strdraw.h"
#include "prefs.h"
#include "initfile.h"
#include "hiscore.h"
#include "gamelib/vfs.h"
#include "gamelib/wav.h"
#include "gamelib/mixer.h"
#include "gamelib/lang.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <stdlib.h>
#include <string.h>

enum {
	LOAD_NEXT_BITMAP,
	LOAD_NEXT_SOUND,
	LOAD_DATA_LOADED,
	LOAD_LAST_STATE
};

static int s_state;

static void draw(void)
{
	unsigned int *pi;
	unsigned char *pb;
	int x, y;

	te_begin_draw();

	pb = te_screen.pixels;
	pi = (unsigned int *) pb;
	for (y = 0; y < te_screen.h; y++) {
		for (x = 0; x < te_screen.w; x++)
			*pi++ = 0x00000000;
		pb += te_screen.pitch;
		pi = (unsigned int *) pb;
	}

	te_end_draw();
}

static void load_next_bitmaps(void)
{
	int r;

	r = load_next_bitmap();
	if (r == E_BITMAPS_LOAD_EOF) {
		s_state++;
	} else if (r != E_BITMAPS_LOAD_OK) {
		ktrace("cannot load bitmap %s", cur_bitmap_name());
		exit(EXIT_FAILURE);
	}
}

static void load_next_sounds(void)
{
	int r;

	r = load_next_sound();
	if (r == E_SOUNDS_LOAD_EOF) {
		s_state++;
	} else if (r != E_SOUNDS_LOAD_OK) {
		ktrace("cannot load sound %s", cur_sound_name());
		exit(EXIT_FAILURE);
	}
}

/*
 * Bitmaps are loaded.
 * We can do other initialization depending on that.
 */
static void on_data_loaded(void)
{
	const char *lang;
	int loaded, save;

	initfile_load();
	if (PP_DEMO && !initfile_getvar("demo_version")) {
		ktrace("This demo version can only load a demo data.pak!");
		exit(EXIT_FAILURE);
	}

	load_lang_list();

	/* Set system language the first time or the saved one. */
	loaded = 0;
	save = 0;
	load_prefs();
	if (strcmp(get_preference("default"), "1") == 0) {
		save = 1;
		lang = get_lang_equivalence(guess_lang());
		if (is_lang_code_listed(lang)) {
			set_preference("lang", lang);
			load_lang(lang);
			loaded = 1;
		}
	}

	if (!loaded) {
		lang = get_preference("lang");
		if (!is_lang_code_listed(lang)) {
			lang = get_lang_equivalence(lang);
			if (!is_lang_code_listed(lang))
				set_preference("lang", "en");
			else
				set_preference("lang", lang);
			save = 1;
		}
		load_lang(get_preference("lang"));
	}

	if (save) {
		save_prefs();
	}

	load_defined_keys();
	hiscore_load();
	mixer_set_volume(atoi(get_preference("volume")));

	if (strcmp(get_preference("stargate"), "1") == 0) {
		apply_stargate_symbols();
	}

	te_set_fg_tileset_bmp(0, bmp_font0);
	te_set_fg_tileset_bmp(1, bmp_font1);
	te_set_bg_blockset(&main_blockset);
	s_state++;
}

static void update(void)
{
	switch (s_state) {
	case LOAD_NEXT_BITMAP: load_next_bitmaps(); break;
	case LOAD_NEXT_SOUND: load_next_sounds(); break;
	case LOAD_DATA_LOADED: on_data_loaded(); break;
	case LOAD_LAST_STATE: switch_to_state(&menu_st); break;
	}
}

static void enter(const struct state *old_state)
{
	vfs_set_base_path(kernel_get_device()->get_data_path());
	ktrace("data path is %s", kernel_get_device()->get_data_path());
	te_set_half_speed_mode(IS_FULL_FPS);
	s_state = 0;
}

const struct state load_st = {
	.enter = enter,
	.update = update,
	.draw = draw,
};
