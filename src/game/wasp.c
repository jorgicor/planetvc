/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "wasp.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_wasp = NULL;

static BBOX(wasp, 7, 15, 17, 6)

static FRAMEB(wasp_fly_0, bmp_wasp, 0, 0, 32, 32, wasp)
static FRAMEB(wasp_fly_1, bmp_wasp, 32, 0, 32, 32, wasp)
static FRAMEB(wasp_fly_2, bmp_wasp, 64, 0, 32, 32, wasp)
static FRAMEB(wasp_fly_3, bmp_wasp, 96, 0, 32, 32, wasp)
static FRAMEB(wasp_fly_4, bmp_wasp, 128, 0, 32, 32, wasp)
static FRAMEB(wasp_fly_5, bmp_wasp, 160, 0, 32, 32, wasp)

static BEGIN_ANIM(wasp_fly)
ANIM_FRAME(wasp_fly_0)
ANIM_FRAME(wasp_fly_1)
ANIM_FRAME(wasp_fly_2)
ANIM_FRAME(wasp_fly_1)
ANIM_FRAME(wasp_fly_3)
ANIM_FRAME(wasp_fly_4)
ANIM_FRAME(wasp_fly_5)
ANIM_FRAME(wasp_fly_4)
END_ANIM

static DEFANIM(wasp_fly)

static void wasp_idle(struct actor *pac)
{
	update_actor_path(pac, 1);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static struct actor *spawn_wasp(int tx, int ty, int dir)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_wasp_fly, 4, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = wasp_idle;

	set_actor_dir(pac, dir);

	return pac;
}

static struct actor *spawn_wasp_fp(int tx, int ty)
{
	int r;
	char dir[5];
	int idir;

	r = tokscanf(NULL, "s", dir, NELEMS(dir));
	if (r != 1) {
		return NULL;
	}

	idir = RDIR;
	if (strcmp(dir, "ldir") == 0) {
		idir = LDIR;
	} else if (strcmp(dir, "rdir") == 0) {
		idir = RDIR;
	}

	return spawn_wasp(tx, ty, idir);
}

void wasp_init(void)
{
	register_bitmap(&bmp_wasp, "wasp", 1, 128);
	register_spawn_fn("wasp", spawn_wasp_fp);
}
