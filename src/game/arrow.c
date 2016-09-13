/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "arrow.h"
#include "bitmaps.h"
#include "tilengin.h"
#include "game.h"
#include "cosmonau.h"
#include "oxigen.h"
#include "data.h"
#include "gplay_st.h"
#include "initfile.h"
#include "msgbox.h"
#include "gamelib/mixer.h"

static struct bmp *bmp_arrow = NULL;

static FRAME(arrow, bmp_arrow, 0, 0, 16, 16)

#define ARROW_COUNT	i0

enum {
	ARROW_TIME = FPS,
	ARROW_BLINK_COUNT = 3,
};

static void arrow_work(struct actor *pac)
{
	pac->t = dectime(pac->t);
	if (pac->t <= 0) {
		if (pac->psp->pframe != NULL) {
			pac->psp->pframe = NULL;
		} else {
			pac->psp->pframe = &fr_arrow;
		}
		pac->t = ARROW_TIME;
		if (pac->psp->pframe != NULL) {
			pac->ARROW_COUNT++;
			if (pac->ARROW_COUNT >= ARROW_BLINK_COUNT) {
				te_free_sprite(pac->psp);
				free_actor(pac);
			} else {
				mixer_play(wav_opsel);
			}
		}
	}
}

static void arrow_idle(struct actor *pac)
{
	struct sprite *psp;

	if (!s_got_oxigen)
		return;

	psp = te_get_sprite(SP_ARROW);
	psp->pframe = &fr_arrow;
	psp->flags = SP_F_TOP;
	pac->t = ARROW_TIME;
	pac->ARROW_COUNT = 0;
	pac->psp = psp;
	pac->update = arrow_work;
	cosmonaut_set_actor_exit_dirpos(pac);
	mixer_play(wav_opsel);
}

static void spawn_arrow(int mapid)
{
	struct actor *pac;

	if (g_state != &gplay_st)
		return;

	if (initfile_getvar("arrow") == 0)
		return;

	pac = get_actor(AC_ARROW);
	pac->update = arrow_idle;
}

void arrow_init(void)
{
	register_bitmap(&bmp_arrow, "arrow", 1, 0x80);
	register_map_init_fn(spawn_arrow);
}
