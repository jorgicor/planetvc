/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "caterpil.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_caterpil = NULL;

static BBOX(caterpillar, 4, 8, 25, 6)

static FRAMEB(caterpillar_walk_0, bmp_caterpil, 0, 0, 32, 16, caterpillar)
static FRAMEB(caterpillar_walk_1, bmp_caterpil, 0, 16, 32, 16, caterpillar)
static FRAMEB(caterpillar_walk_2, bmp_caterpil, 0, 32, 32, 16, caterpillar)
static FRAMEB(caterpillar_walk_3, bmp_caterpil, 0, 48, 32, 16, caterpillar)
static FRAMEB(caterpillar_walk_4, bmp_caterpil, 0, 64, 32, 16, caterpillar)

static BEGIN_ANIM(caterpillar_walk)
ANIM_FRAME(caterpillar_walk_0)
ANIM_FRAME(caterpillar_walk_1)
ANIM_FRAME(caterpillar_walk_2)
ANIM_FRAME(caterpillar_walk_3)
ANIM_FRAME(caterpillar_walk_4)
END_ANIM

static DEFANIM(caterpillar_walk)

static void caterpillar_walk(struct actor *pac)
{
	update_actor_path(pac, 1);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static struct actor *spawn_caterpillar(int tx, int ty, int flipv)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	psp->flags = (flipv) ? SP_F_FLIPV : 0;
	te_set_anim(psp, &am_caterpillar_walk, 4, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = caterpillar_walk;

	return pac;
}

static struct actor *spawn_caterpillar_fp(int tx, int ty)
{
	char dir[6];
	int r, flipv;

	r = tokscanf(NULL, "s", dir, NELEMS(dir));
	if (r != 1) {
		return NULL;
	}

	flipv = (strcmp(dir, "flipv") == 0);
	return spawn_caterpillar(tx, ty, flipv);
}

void caterpil_init(void)
{
	register_bitmap(&bmp_caterpil, "caterpil", 1, 128);
	register_spawn_fn("caterpillar", spawn_caterpillar_fp);
}
