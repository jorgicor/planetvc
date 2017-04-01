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

#include "fish.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "sounds.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <string.h>

static struct bmp *bmp_fish = NULL;
static struct bmp *bmp_fishshot = NULL;
static struct wav *wav_fishshot = NULL;

static BBOX(fish, 0, 0, 32, 32)

static FRAMEB(fish_0, bmp_fish, 0, 0, 32, 32, fish)
static FRAMEB(fish_1, bmp_fish, 32, 0, 32, 32, fish)
static FRAMEB(fish_2, bmp_fish, 64, 0, 32, 32, fish)
static FRAMEB(fish_3, bmp_fish, 0, 32, 32, 32, fish)
static FRAMEB(fish_4, bmp_fish, 32, 32, 32, 32, fish)
static FRAMEB(fish_5, bmp_fish, 64, 32, 32, 32, fish)

static BEGIN_ANIM(fish_idle)
ANIM_FRAME(fish_3)
ANIM_FRAME(fish_4)
ANIM_FRAME(fish_5)
ANIM_FRAME(fish_4)
END_ANIM

static BEGIN_ANIM(fish_shot)
ANIM_FRAME(fish_0)
ANIM_FRAME(fish_1)
ANIM_FRAME(fish_2)
ANIM_FRAME(fish_1)
END_ANIM

static DEFANIM(fish_idle)
static DEFANIM(fish_shot)

static BBOX(fishshot, 14, 12, 6, 6)

static FRAMEB(fishshot_s0, bmp_fishshot, 0, 0, 32, 32, fishshot)
static FRAMEB(fishshot_s1, bmp_fishshot, 32, 0, 32, 32, fishshot)
static FRAMEB(fishshot_s2, bmp_fishshot, 64, 0, 32, 32, fishshot)
static FRAMEB(fishshot_f0, bmp_fishshot, 96, 0, 32, 32, fishshot)
static FRAMEB(fishshot_f1, bmp_fishshot, 128, 0, 32, 32, fishshot)

static BEGIN_ANIM(fishshot_start)
ANIM_FRAME(fishshot_s0)
ANIM_FRAME(fishshot_s1)
ANIM_FRAME(fishshot_s2)
END_ANIM

static BEGIN_ANIM(fishshot_fly)
ANIM_FRAME(fishshot_f0)
ANIM_FRAME(fishshot_f1)
END_ANIM

static DEFANIM(fishshot_start)
static DEFANIM(fishshot_fly)

#define START_TIME	i0
#define VY		vy
#define AY		ay
	
#define SHOT_STARTY	i0

enum {
	ANIM_SPEED = 4
};

static void fishshot_run(struct actor *pac)
{
	int oldvy;

	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}

	oldvy = pac->vy;
	move_actor_y(pac);
	if (pac->psp->panim == &am_fishshot_start &&
		!te_is_anim_playing(pac->psp))
       	{
		te_set_anim(pac->psp, &am_fishshot_fly, 4, 1);
	}
	if (oldvy < 0 && pac->vy >= 0) {
		/* We start falling */
		set_actor_dir(pac, DDIR);
		te_set_anim(pac->psp, &am_fishshot_start, 4, 0);
	}
	if (pac->psp->y >= pac->SHOT_STARTY) {
		te_free_sprite(pac->psp);
		free_actor(pac);
	}
}

static void spawn_fishshot(int x, int y, int vy, int ay)
{
	struct actor *pac;
	struct sprite *psp;

	psp = te_get_free_sprite(SP_ESHOT_0, SP_ESHOT_LAST + 1);
	if (psp == NULL)
		return;
	pac = get_free_actor(AC_ESHOT_0, AC_ESHOT_LAST + 1);
	if (pac == NULL) {
		te_free_sprite(psp);
		return;
	}

	psp->x = x;
	psp->y = y;
	te_set_anim(psp, &am_fishshot_start, 4, 0);
	pac->psp = psp;
	pac->vy = VY;
	pac->ay = AY;
	pac->SHOT_STARTY = psp->y;
	pac->update = fishshot_run;

	mixer_play(wav_fishshot);
}

static void fish_idle(struct actor *pac)
{
	if (pac->psp->panim == &am_fish_shot) {
		if (!te_is_anim_playing(pac->psp)) {
			te_set_anim(pac->psp, &am_fish_idle, 4, 1);
			pac->t = pac->START_TIME;
		}
	} else {
		pac->t = dectime(pac->t);
		if (pac->t <= 0) {
			te_set_anim(pac->psp, &am_fish_shot, 4, 0);
			spawn_fishshot(pac->psp->x, pac->psp->y,
				       pac->VY, pac->AY);
		}
	}
}

static struct actor *spawn_fish(int tx, int ty, int dir, int frames,
				int vy, int ay)
{
	struct sprite *psp;
	struct actor *pac;

	if (vy > 0)
		vy = 0;
	if (ay < 0)
		ay = 1;
	if (frames < ANIM_SPEED * 4)
		frames = ANIM_SPEED * 4;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_fish_idle, ANIM_SPEED, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = fish_idle;
	pac->START_TIME = frames * 2;
	pac->VY = vy;
	pac->AY = ay;
	pac->t = pac->START_TIME;
	set_actor_dir(pac, dir);
	return pac;
}

static struct actor *spawn_fish_fp(int tx, int ty)
{
	int r;
	char dir[5];
	int idir, frames, vy, ay;

	r = tokscanf(NULL, "siii", dir, NELEMS(dir), &frames, &vy, &ay);
	if (r != 4) {
		return NULL;
	}

	idir = RDIR;
	if (strcmp(dir, "ldir") == 0) {
		idir = LDIR;
	} else if (strcmp(dir, "rdir") == 0) {
		idir = RDIR;
	}

	return spawn_fish(tx, ty, idir, frames, vy, ay);
}

void fish_init(void)
{
	if (PP_DEMO)
		return;
	register_bitmap(&bmp_fish, "fish", 1, 0);
	register_bitmap(&bmp_fishshot, "fishshot", 1, 0);
	register_spawn_fn("fish", spawn_fish_fp);
	register_sound(&wav_fishshot, "fishshot");
}
