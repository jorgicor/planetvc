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

#include "fenix.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_fenix = NULL;

static BBOX(fenix, 7, 14, 18, 8)

static FRAMEB(fenix_fly_0, bmp_fenix, 0, 0, 32, 32, fenix)
static FRAMEB(fenix_fly_1, bmp_fenix, 32, 0, 32, 32, fenix)
static FRAMEB(fenix_fly_2, bmp_fenix, 64, 0, 32, 32, fenix)
static FRAMEB(fenix_fly_3, bmp_fenix, 96, 0, 32, 32, fenix)

static BEGIN_ANIM(fenix_fly)
ANIM_FRAME(fenix_fly_0)
ANIM_FRAME(fenix_fly_1)
ANIM_FRAME(fenix_fly_2)
ANIM_FRAME(fenix_fly_3)
END_ANIM

static DEFANIM(fenix_fly)

static void fenix_idle(struct actor *pac)
{
	update_actor_path(pac, 1);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static struct actor *spawn_fenix(int tx, int ty, int dir)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_fenix_fly, 4, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = fenix_idle;
	set_actor_dir(pac, dir);
	return pac;
}

static struct actor *spawn_fenix_fp(int tx, int ty)
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

	return spawn_fenix(tx, ty, idir);
}

void fenix_init(void)
{
	register_bitmap(&bmp_fenix, "fenix", 1, 128);
	register_spawn_fn("fenix", spawn_fenix_fp);
}
