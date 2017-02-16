/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif

#ifdef DEBUG
#ifdef HAVE_MCHECK_H
#include <mcheck.h>
#endif
#endif

#include "game.h"
#include "bitmaps.h"
#include "sounds.h"
#include "data.h"
#include "balloon.h"
#include "cosmonau.h"
#include "oxigen.h"
#include "wasp.h"
#include "caterpil.h"
#include "spider.h"
#include "platform.h"
#include "drop.h"
#include "moth.h"
#include "grasshop.h"
#include "lavatong.h"
#include "stargate.h"
#include "bat.h"
#include "fish.h"
#include "fenix.h"
#include "draco.h"
#include "lavashot.h"
#include "hud.h"
#include "menu_st.h"
#include "gameover.h"
#include "hiscore.h"
#include "arrow.h"
#include "pad.h"
#include "text.h"
#include "prefs.h"
#include "strdraw.h"
#include "modplay.h"
#include "msgbox.h"
#include "demosave.h"
#include "confpath.h"
#include "log.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include <SDL.h>

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef TIME_H
#define TIME_H
#include <time.h>
#endif

int main(int argc, char *argv[])
{
#ifdef DEBUG
#ifdef HAVE_MCHECK_H
	mtrace();
#endif
#endif

	/*
	 * All modules are initialized here.
	 */

	srand(time(0));

	/* Log to console always. */
	kassert_set_log_fun(kernel_get_device()->trace);

	confpath_init();
	log_init();
	text_init();
	prefs_init();
	mixer_init();
	data_init();
	strdraw_init();
	msgbox_init();
	balloon_init();
	cosmonau_init();
	oxigen_init();
	wasp_init();
	caterpil_init();
	spider_init();
	platform_init();
	drop_init();
	moth_init();
	grasshop_init();
	lavatong_init();
	stargate_init();
	bat_init();
	fish_init();
	fenix_init();
	draco_init();
	lavashot_init();
	hud_init();
	menu_st_init();
	gameover_init();
	hiscore_init();
	demosave_init();
	arrow_init();
	pad_init();

	/* Main loop */

	game_run();

	/*
	 * All modules are de-initialized here.
	 */

	modplay_done();
	mixer_done();
	bitmaps_done();
	sounds_done();
	log_done();

	return EXIT_SUCCESS;
}
