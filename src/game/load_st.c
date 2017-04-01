/*
Copyright (c) 2014-2017 Jorge Giner Cordero

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
#include "coroutin.h"
#include "google.h"
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
	LOAD_GOOGLE_CONNECT,
	LOAD_GOOGLE_CONNECTING,
	LOAD_LAST_STATE
};

static int s_state;

int s_google_autoconnect_performed;

enum {
	POINTW = 3
};

static void draw_point(int x, int y, int color)
{
	unsigned char *pb;
	unsigned int *pi;

	if (x < 0 || x > te_screen.w - POINTW)
		return;
	if (y < 0 || y > te_screen.h - POINTW)
		return;
	pb = te_screen.pixels + y * te_screen.pitch;
	pi = (unsigned int *) pb;
	pi += x;
	for (y = 0; y < POINTW; y++) {
		for (x = 0; x < POINTW; x++) {
			*(pi + x) = color;
		}
		pb = (unsigned char *) pi;
		pb += te_screen.pitch;
		pi = (unsigned int *) pb;
	}
}

static void draw_loading(void)
{
	int x, y, w, i;

	y = (te_screen.h - POINTW) / 2;
	w = (POINTW + 1) * LOAD_LAST_STATE;
	x = (te_screen.w - w) / 2;
	for (i = 0; i < LOAD_LAST_STATE; i++) {
		if (i >= s_state) {
			draw_point(x, y, 0x888888);
		} else {
			draw_point(x, y, 0xffffff);
		}
		x += POINTW + 1;
	}
}

static void draw(void)
{
	unsigned char *pb;
	int y;

	te_begin_draw();

	pb = te_screen.pixels;
	for (y = 0; y < te_screen.h; y++) {
		memset(pb, 0, te_screen.w * sizeof(unsigned int));
		pb += te_screen.pitch;
	}

	draw_loading();

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
	if (preference_equals("default", "1")) {
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

	if (preference_equals("stargate", "1")) {
		apply_stargate_symbols();
	}

	te_set_fg_tileset_bmp(0, bmp_font0);
	te_set_fg_tileset_bmp(1, bmp_font1);
	te_set_bg_blockset(&main_blockset);
	s_state++;
}

static int google_connect_cr(int restart)
{
	crBegin(restart);
	Android_ConnectToGooglePlay();
	while (Android_IsConnectingToGooglePlay()) {
		crReturn(0);
	}
	crFinish;
	return 1;
}

static void google_connect(void)
{
	s_google_autoconnect_performed = 0;
	if (preference_equals("connectq", "1") &&
	    preference_equals("autoconnect", "1"))
	{
		s_google_autoconnect_performed = 1;
		start_coroutine(google_connect_cr);
	}
	s_state++;
}

static void google_connecting(void)
{
	if (is_coroutine_running()) {
		run_coroutine();
	} else {
		s_state++;
	}
}

static void update(void)
{
	switch (s_state) {
	case LOAD_NEXT_BITMAP: load_next_bitmaps(); break;
	case LOAD_NEXT_SOUND: load_next_sounds(); break;
	case LOAD_DATA_LOADED: on_data_loaded(); break;
	case LOAD_GOOGLE_CONNECT: google_connect(); break;
	case LOAD_GOOGLE_CONNECTING: google_connecting(); break;
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
