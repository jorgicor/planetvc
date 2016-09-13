/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "gamelib/mixer.h"

static struct bmp *bmp_oxigen = NULL;
static struct wav *wav_oxigen = NULL;

static BBOX(oxigen, 1, 1, 13, 13)

FRAMEB(oxigen_shine_0, bmp_oxigen, 0, 0, 16, 16, oxigen)
static FRAMEB(oxigen_shine_1, bmp_oxigen, 16, 0, 16, 16, oxigen)
static FRAMEB(oxigen_shine_2, bmp_oxigen, 32, 0, 16, 16, oxigen)
static FRAMEB(oxigen_shine_3, bmp_oxigen, 48, 0, 16, 16, oxigen)
static FRAMEB(oxigen_shine_4, bmp_oxigen, 64, 0, 16, 16, oxigen)

static FRAME(oxigen_explode_0, bmp_oxigen, 80, 0, 16, 16)
static FRAME(oxigen_explode_1, bmp_oxigen, 96, 0, 16, 16)
static FRAME(oxigen_explode_2, bmp_oxigen, 112, 0, 16, 16)

static BEGIN_ANIM(oxigen_shine)
ANIM_FRAME(oxigen_shine_0)
ANIM_FRAME(oxigen_shine_1)
ANIM_FRAME(oxigen_shine_2)
ANIM_FRAME(oxigen_shine_3)
ANIM_FRAME(oxigen_shine_4)
ANIM_FRAME(oxigen_shine_0)
END_ANIM

static BEGIN_ANIM(oxigen_explode)
ANIM_FRAME(oxigen_explode_0)
ANIM_FRAME(oxigen_explode_1)
ANIM_FRAME(oxigen_explode_2)
END_ANIM

static DEFANIM(oxigen_shine)
static DEFANIM(oxigen_explode)

enum {
	TIME_TO_SHINE = FPS * 5,
	VY = -TE_ONE / 4,
	AY = TE_ONE / 64
};

/* If we have gathered an oxigen bubble or not. */
int s_got_oxigen;

static void oxigen_explode(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		te_free_sprite(pac->psp);
		free_actor(pac);
	}
}

static void explode(struct actor *pac)
{
	mixer_play(wav_oxigen);
	te_set_anim(pac->psp, &am_oxigen_explode, 2, 0);
	pac->update = oxigen_explode;
}

static void oxigen_idle(struct actor *pac)
{
	struct sprite *psp;

	psp = pac->psp;
	if (psp->panim == NULL) {
		pac->t--;
		if (pac->t <= 0)
			te_set_anim(psp, &am_oxigen_shine, 2, 0);
	} else if (!te_is_anim_playing(psp)) {
		psp->panim = NULL;
		psp->pframe = &fr_oxigen_shine_0;
		pac->t = TIME_TO_SHINE;
	}

	move_actor_y(pac);
	if (pac->ay > 0) {
		if (psp->y >= pac->i0) {
			psp->y = pac->i0;
			pac->vy = -VY;
			pac->ay = -pac->ay;
		}
	} else if (pac->ay < 0) {
		if (psp->y <= pac->i0) {
			psp->y = pac->i0;
			pac->vy = VY;
			pac->ay = -pac->ay;
		}
	}

	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		s_got_oxigen = 1;
		explode(pac);
	}
}

struct actor *spawn_oxigen(int tx, int ty)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_OXIGEN_0, SP_OXIGEN_LAST + 1);
	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW - (TE_VBTW / 2) + (TE_ONE / 2); 
	te_set_anim(psp, &am_oxigen_shine, 2, 0);

	pac = get_free_actor(AC_OXIGEN_0, AC_OXIGEN_LAST + 1);
	pac->psp = psp;
	pac->t = TIME_TO_SHINE;
	pac->vy = VY;
	pac->ay = AY;
	pac->i0 = psp->y;
	pac->update = oxigen_idle;

	return pac;
}

static struct actor *spawn_oxigen_fp(int tx, int ty)
{
	return spawn_oxigen(tx, ty);
}

void oxigen_init(void)
{
	register_bitmap(&bmp_oxigen, "oxigen", 1, 0);
	register_spawn_fn("oxigen", spawn_oxigen_fp);
	register_sound(&wav_oxigen, "oxigen");
}
