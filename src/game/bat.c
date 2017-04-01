/*
Copyright (c) 2016-2017 Jorge Giner Cordero

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

#include "bat.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_bat = NULL;

static BBOX(bat_0, 10, 11, 13, 8)
static BBOX(bat_1, 10, 11, 13, 8)
static BBOX(bat_2, 10, 11, 13, 8)

static FRAMEB(bat_fly_0, bmp_bat, 0, 0, 32, 32, bat_0)
static FRAMEB(bat_fly_1, bmp_bat, 32, 0, 32, 32, bat_1)
static FRAMEB(bat_fly_2, bmp_bat, 64, 0, 32, 32, bat_2)

static BEGIN_ANIM(bat_fly)
ANIM_FRAME(bat_fly_0)
ANIM_FRAME(bat_fly_1)
ANIM_FRAME(bat_fly_2)
END_ANIM

static DEFANIM(bat_fly)

static void bat_idle(struct actor *pac)
{
	update_actor_path(pac, 1);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static struct actor *spawn_bat(int tx, int ty, int dir)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_bat_fly, 4, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = bat_idle;
	set_actor_dir(pac, dir);
	return pac;
}

static struct actor *spawn_bat_fp(int tx, int ty)
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

	return spawn_bat(tx, ty, idir);
}

void bat_init(void)
{
	register_bitmap(&bmp_bat, "bat", 1, 128);
	register_spawn_fn("bat", spawn_bat_fp);
}
