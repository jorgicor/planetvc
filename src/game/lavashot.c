/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "lavashot.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "cosmonau.h"
#include "debug.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"

static struct bmp *bmp_lavashot = NULL;
static struct wav *wav_lavashot = NULL;

static BBOX(lavashot, 5, 14, 5, 5)

static FRAME(lavashot_idle_0, bmp_lavashot, 0, 0, 16, 32)
static FRAME(lavashot_idle_1, bmp_lavashot, 16, 0, 16, 32)
static FRAME(lavashot_idle_2, bmp_lavashot, 32, 0, 16, 32)
static FRAMEB(lavashot_jump_0, bmp_lavashot, 48, 0, 16, 32, lavashot)
static FRAMEB(lavashot_jump_1, bmp_lavashot, 64, 0, 16, 32, lavashot)
static FRAMEB(lavashot_jump_2, bmp_lavashot, 80, 0, 16, 32, lavashot)
static FRAMEB(lavashot_jump_3, bmp_lavashot, 96, 0, 16, 32, lavashot)

static BEGIN_ANIM(lavashot_idle)
ANIM_FRAME(lavashot_idle_0)
ANIM_FRAME(lavashot_idle_1)
ANIM_FRAME(lavashot_idle_2)
ANIM_FRAME(lavashot_idle_1)
END_ANIM

static BEGIN_ANIM(lavashot_jump)
ANIM_FRAME(lavashot_jump_0)
ANIM_FRAME(lavashot_jump_1)
ANIM_FRAME(lavashot_jump_2)
ANIM_FRAME(lavashot_jump_3)
END_ANIM

static DEFANIM(lavashot_idle)
static DEFANIM(lavashot_jump)

#define START_VY	vx
#define START_AY	ax
#define START_TIME	i0
#define START_Y		i1

static void set_idle(struct actor *pac);

static void lavashot_jump(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	move_actor_y(pac);
	if (pac->psp->y >= pac->START_Y) {
		mixer_play(wav_lavashot);
		set_idle(pac);
	}
}

static void set_jump(struct actor *pac)
{
	pac->vy = pac->START_VY;
	pac->ay = pac->START_AY;
	te_set_anim(pac->psp, &am_lavashot_jump, 4, 1);
	pac->update = lavashot_jump;
}

static void lavashot_idle(struct actor *pac)
{
	pac->t = dectime(pac->t);
	if (pac->t <= 0) {
		set_jump(pac);
	}
}

static void set_idle(struct actor *pac)
{
	pac->t = pac->START_TIME;
	pac->psp->y = pac->START_Y;
	te_set_anim(pac->psp, &am_lavashot_idle, 4, 1);
	pac->update = lavashot_idle;
}

/* two_tiles: if we must spawn in the middle of two tiles. */
static struct actor *spawn_lavashot(int tx, int ty, int two_tiles, int frames,
				    int vy, int ay)
{
	struct actor *pac;
	struct sprite *psp;

	psp = te_get_free_sprite(SP_ESHOT_0, SP_ESHOT_LAST + 1);
	if (psp == NULL)
		return NULL;
	pac = get_free_actor(AC_ESHOT_0, AC_ESHOT_LAST + 1);
	if (pac == NULL) {
		te_free_sprite(psp);
		return NULL;
	}

	if (vy > 0)
		vy = 0;
	if (ay < 0)
		ay = 1;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW;
	if (two_tiles)
		psp->x += TE_VBTW / 2;
	pac->psp = psp;
	pac->START_Y = psp->y;
	pac->START_VY = vy;
	pac->START_AY = ay;
	pac->START_TIME = frames * 2;
	set_idle(pac);
	return pac;
}

static struct actor *spawn_lavashot_fp(int tx, int ty)
{
	int r;
	int two_tiles, frames, vy, ay;

	r = tokscanf(NULL, "iiii", &two_tiles, &frames, &vy, &ay);
	if (r != 4) {
		return NULL;
	}

	return spawn_lavashot(tx, ty, two_tiles, frames, vy, ay);
}

void lavashot_init(void)
{
	if (DEMO_ON)
		return;
	register_bitmap(&bmp_lavashot, "lavashot", 1, 0);
	register_spawn_fn("lavashot", spawn_lavashot_fp);
	register_sound(&wav_lavashot, "lavashot");
}
