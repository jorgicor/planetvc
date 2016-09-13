/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "drop.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "cosmonau.h"
#include "platform.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_drop = NULL;
static struct wav *wav_drop = NULL;

static BBOX(drop_0, 5, 4, 5, 7)
static BBOX(drop_1, 4, 5, 7, 6)
static BBOX(drop_2, 3, 6, 9, 6)
static BBOX(drop_3, 3, 4, 10, 6)
static BBOX(drop_4, 3, 5, 10, 5)
static BBOX(drop_5, 3, 7, 10, 4)

static FRAMEB(drop_fall0_0, bmp_drop, 0, 0, 16, 16, drop_0)
static FRAMEB(drop_hit0_0, bmp_drop, 0, 16, 16, 16, drop_1)
static FRAMEB(drop_hit0_1, bmp_drop, 0, 32, 16, 16, drop_2)
static FRAMEB(drop_hit0_2, bmp_drop, 0, 48, 16, 16, drop_3)
static FRAMEB(drop_hit0_3, bmp_drop, 0, 64, 16, 16, drop_4)
static FRAMEB(drop_hit0_4, bmp_drop, 0, 80, 16, 16, drop_5)

static BEGIN_ANIM(drop_fall_c0)
ANIM_FRAME(drop_fall0_0)
END_ANIM

static BEGIN_ANIM(drop_hit_c0)
ANIM_FRAME(drop_hit0_0)
ANIM_FRAME(drop_hit0_1)
ANIM_FRAME(drop_hit0_2)
ANIM_FRAME(drop_hit0_3)
ANIM_FRAME(drop_hit0_4)
END_ANIM

static DEFANIM(drop_fall_c0)
static DEFANIM(drop_hit_c0)

static FRAMEB(drop_fall1_0, bmp_drop, 16, 0, 16, 16, drop_0)
static FRAMEB(drop_hit1_0, bmp_drop, 16, 16, 16, 16, drop_1)
static FRAMEB(drop_hit1_1, bmp_drop, 16, 32, 16, 16, drop_2)
static FRAMEB(drop_hit1_2, bmp_drop, 16, 48, 16, 16, drop_3)
static FRAMEB(drop_hit1_3, bmp_drop, 16, 64, 16, 16, drop_4)
static FRAMEB(drop_hit1_4, bmp_drop, 16, 80, 16, 16, drop_5)

static BEGIN_ANIM(drop_fall_c1)
ANIM_FRAME(drop_fall1_0)
END_ANIM

static BEGIN_ANIM(drop_hit_c1)
ANIM_FRAME(drop_hit1_0)
ANIM_FRAME(drop_hit1_1)
ANIM_FRAME(drop_hit1_2)
ANIM_FRAME(drop_hit1_3)
ANIM_FRAME(drop_hit1_4)
END_ANIM

static DEFANIM(drop_fall_c1)
static DEFANIM(drop_hit_c1)

static FRAMEB(drop_fall2_0, bmp_drop, 32, 0, 16, 16, drop_0)
static FRAMEB(drop_hit2_0, bmp_drop, 32, 16, 16, 16, drop_1)
static FRAMEB(drop_hit2_1, bmp_drop, 32, 32, 16, 16, drop_2)
static FRAMEB(drop_hit2_2, bmp_drop, 32, 48, 16, 16, drop_3)
static FRAMEB(drop_hit2_3, bmp_drop, 32, 64, 16, 16, drop_4)
static FRAMEB(drop_hit2_4, bmp_drop, 32, 80, 16, 16, drop_5)

static BEGIN_ANIM(drop_fall_c2)
ANIM_FRAME(drop_fall2_0)
END_ANIM

static BEGIN_ANIM(drop_hit_c2)
ANIM_FRAME(drop_hit2_0)
ANIM_FRAME(drop_hit2_1)
ANIM_FRAME(drop_hit2_2)
ANIM_FRAME(drop_hit2_3)
ANIM_FRAME(drop_hit2_4)
END_ANIM

static DEFANIM(drop_fall_c2)
static DEFANIM(drop_hit_c2)

enum {
	NDROP_COLORS = 3
};

/* Animations for each color. */
static struct anim *s_drop_fall_anims[] = {
	&am_drop_fall_c0,
	&am_drop_fall_c1,
	&am_drop_fall_c2,
};

static struct anim *s_drop_hit_anims[] = {
	&am_drop_hit_c0,
	&am_drop_hit_c1,
	&am_drop_hit_c2,
};

#define START_Y		i0
#define COLOR		i1
#define START_AY	t

static void drop_fall(struct actor *pac);
static void drop_hit(struct actor *pac);

static void drop_hit(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		pac->psp->y = pac->START_Y;
		pac->ay = pac->START_AY;
		pac->vy = 0;
		te_set_anim(pac->psp, s_drop_fall_anims[pac->COLOR], 0, 0);
		pac->update = drop_fall;
	}
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static void drop_set_hit(struct actor *pac)
{
	mixer_play(wav_drop);
	te_set_anim(pac->psp, s_drop_hit_anims[pac->COLOR], 4, 0);
	pac->update = drop_hit;
}

static void drop_fall(struct actor *pac)
{
	int oy, od, d;
	struct rect box;

	if (kassert_fails(pac->psp != NULL &&
			  pac->psp->pframe != NULL &&
			  pac->psp->pframe->pbbox != NULL))
	{
		return;
	}

	oy = pac->psp->y;
	move_actor_y(pac);
	od = pac->psp->y - oy;
	pac->psp->y = oy;
	get_actor_bbox(&box, pac);
	d = how_far_y(box.x, box.y, box.x + box.w - 1, box.y + box.h - 1, od);
	if (d != 0) {
		d = how_far_y_platforms(box.x, box.y, box.x + box.w - 1,
			box.y + box.h - 1, d);
	}
	pac->psp->y += d;
	if (od != d) {
		drop_set_hit(pac);
	}
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static struct actor *spawn_drop(int tx, int ty, int accel, int color)
{
	struct sprite *psp;
	struct actor *pac;

	if (kassert_fails(color < NDROP_COLORS))
		color = 0;

	kasserta(color < NDROP_COLORS);

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, s_drop_fall_anims[color], 0, 0);
	psp->y -= te_p2w(psp->pframe->pbbox->y);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = drop_fall;
	pac->ay = accel;
	pac->START_Y = psp->y;
	pac->START_AY = accel;
	pac->COLOR = color;

	return pac;
}

static struct actor *spawn_drop_fp(int tx, int ty)
{
	int r, accel, color;

	r = tokscanf(NULL, "i", &accel);
	if (r != 1) {
		return NULL;
	}

	r = tokscanf(NULL, "i", &color);
	if (r != 1 || color < 0 || color >= NDROP_COLORS) {
		color = 0;
	}

	return spawn_drop(tx, ty, accel, color);
}

void drop_init(void)
{
	kasserta(NDROP_COLORS == NELEMS(s_drop_fall_anims));
	kasserta(NDROP_COLORS == NELEMS(s_drop_hit_anims));
	register_bitmap(&bmp_drop, "drop", 1, 0);
	register_spawn_fn("drop", spawn_drop_fp);
	register_sound(&wav_drop, "drop");
}
