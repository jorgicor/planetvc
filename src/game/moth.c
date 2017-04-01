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

#include "moth.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "cosmonau.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_moth = NULL;
static struct bmp *bmp_mothray = NULL;
static struct wav *wav_mothray = NULL;

static BBOX(moth, 6, 16, 17, 11)

static FRAMEB(moth_idle_0, bmp_moth, 0, 0, 32, 32, moth)
static FRAMEB(moth_idle_1, bmp_moth, 32, 0, 32, 32, moth)
static FRAMEB(moth_idle_2, bmp_moth, 64, 0, 32, 32, moth)

static FRAMEB(moth_angry_0, bmp_moth, 0, 32, 32, 32, moth)
static FRAMEB(moth_angry_1, bmp_moth, 32, 32, 32, 32, moth)
static FRAMEB(moth_angry_2, bmp_moth, 64, 32, 32, 32, moth)

static BEGIN_ANIM(moth_idle)
ANIM_FRAME(moth_idle_0)
ANIM_FRAME(moth_idle_1)
ANIM_FRAME(moth_idle_2)
ANIM_FRAME(moth_idle_1)
END_ANIM

static BEGIN_ANIM(moth_angry)
ANIM_FRAME(moth_angry_0)
ANIM_FRAME(moth_angry_1)
ANIM_FRAME(moth_angry_2)
ANIM_FRAME(moth_angry_1)
END_ANIM

static DEFANIM(moth_idle)
static DEFANIM(moth_angry)

static BBOX(mothray, 5, 2, 6, 46)

static FRAMEB(mothray_idle_0, bmp_mothray, 0, 0, 16, 48, mothray)
static FRAMEB(mothray_idle_1, bmp_mothray, 16, 0, 16, 48, mothray)
static FRAMEB(mothray_idle_2, bmp_mothray, 32, 0, 16, 48, mothray)

static BEGIN_ANIM(mothray_idle)
ANIM_FRAME(mothray_idle_0)
ANIM_FRAME(mothray_idle_1)
ANIM_FRAME(mothray_idle_2)
ANIM_FRAME(mothray_idle_1)
END_ANIM

static DEFANIM(mothray_idle)

enum {
	MOTH_ANIM_SPEED = 2,
	MOTHRAY_ANIM_SPEED = 4,
	MOTHRAY_DXR = 20 - 8,
	MOTHRAY_DXL = 32 - 20 - 8,
};

static int s_snd_channel = -1;
static int s_moths_playing;

static void moth_idle(struct actor *pac);
static void moth_angry(struct actor *pac);

static void set_ray_xpos(struct actor *pray)
{
	int dx;

	pray->psp->x = pray->pac2->psp->x;
	dx = (pray->pac2->dir == RDIR) ? MOTHRAY_DXR : MOTHRAY_DXL; 
	pray->psp->x += te_p2w(dx);
}

static void mothray_idle(struct actor *pac)
{
	set_ray_xpos(pac);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static struct actor *spawn_mothray(struct actor *pmoth)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ESHOT_0, SP_ESHOT_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->y = pmoth->psp->y + te_p2w(32); 
	te_set_anim(psp, &am_mothray_idle, MOTHRAY_ANIM_SPEED, 1);

	pac = get_free_actor(AC_ESHOT_0, AC_ESHOT_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = mothray_idle;
	pac->pac2 = pmoth;

	set_ray_xpos(pac);

	s_moths_playing++;
	if (s_moths_playing == 1) {
		s_snd_channel = mixer_get_free_channel();
		if (s_snd_channel >= 0) {
			mixer_queue(s_snd_channel, wav_mothray, 1);
		}
	}

	return pac;
}

static void set_angry(struct actor *pac)
{
	int framei;

	framei = pac->psp->framei;
	te_set_anim(pac->psp, &am_moth_angry, pac->psp->speed, 1);
	te_set_anim_frame(pac->psp, framei);
	pac->update = moth_angry;
	pac->pac2 = spawn_mothray(pac);
}

static void set_idle(struct actor *pac)
{
	int framei;

	framei = pac->psp->framei;
	te_set_anim(pac->psp, &am_moth_idle, pac->psp->speed, 1);
	te_set_anim_frame(pac->psp, framei);
	pac->update = moth_idle;
	if (pac->pac2 != NULL) {
		te_free_sprite(pac->pac2->psp);
		free_actor(pac->pac2);
		pac->pac2 = NULL;
		s_moths_playing--;
		if (s_moths_playing == 0) {
			mixer_stop(s_snd_channel);
			s_snd_channel = -1;
		}
	}
}

static void moth_angry(struct actor *pac)
{
	int i, to_idle;
	struct actor *pac2;

	update_actor_path(pac, 1);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}


	to_idle = 1;
	for (i = AC_ENEMY_0; i < AC_ENEMY_LAST + 1; i++) {
		pac2 = get_actor(i);
		if (pac == pac2 || is_free_actor(pac2))
			continue;

		if (pac2->update == moth_idle || pac2->update == moth_angry) {
			if (actor_overlaps(pac, pac2)) {
				to_idle = 0;
				break;
			}
		}
	}

	if (to_idle) {
		set_idle(pac);
	}
}

static void moth_idle(struct actor *pac)
{
	int i;
	struct actor *pac2;

	update_actor_path(pac, 1);
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}

	for (i = AC_ENEMY_0; i < AC_ENEMY_LAST + 1; i++) {
		pac2 = get_actor(i);
		if (pac == pac2 || is_free_actor(pac2))
			continue;

		if (pac2->update == moth_idle || pac2->update == moth_angry) {
			if (actor_overlaps(pac, pac2)) {
				set_angry(pac);
				break;
			}
		}
	}
}

static struct actor *spawn_moth(int tx, int ty)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_moth_idle, MOTH_ANIM_SPEED, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = moth_idle;

	return pac;
}

static struct actor *spawn_moth_fp(int tx, int ty)
{
	return spawn_moth(tx, ty);
}

static void moth_on_map_init(int mapid)
{
	if (s_snd_channel >= 0) {
		mixer_stop(s_snd_channel);
		s_moths_playing = 0;
		s_snd_channel = -1;
	}
}

void moth_init(void)
{
	register_bitmap(&bmp_moth, "moth", 1, 128);
	register_bitmap(&bmp_mothray, "mothray", 1, 0);
	register_spawn_fn("moth", spawn_moth_fp);
	register_map_init_fn(moth_on_map_init);
	register_sound(&wav_mothray, "mothray");
}
