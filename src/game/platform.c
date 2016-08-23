#include "platform.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_platfor1 = NULL;
static struct bmp *bmp_cloud1 = NULL;

static BBOX(platform1, 8, 0, 32, 24)

static FRAMEB(platform1_idle_0, bmp_platfor1, 0, 0, 48, 24, platform1)
static FRAMEB(platform1_idle_1, bmp_platfor1, 0, 24, 48, 24, platform1)
static FRAMEB(platform1_idle_2, bmp_platfor1, 0, 48, 48, 24, platform1)
static FRAMEB(platform1_idle_3, bmp_platfor1, 0, 72, 48, 24, platform1)
static FRAMEB(platform1_idle_4, bmp_platfor1, 0, 96, 48, 24, platform1)
static FRAMEB(platform1_idle_5, bmp_platfor1, 0, 120, 48, 24, platform1)
static FRAMEB(platform1_idle_6, bmp_platfor1, 0, 144, 48, 24, platform1)
static FRAMEB(platform1_idle_7, bmp_platfor1, 0, 168, 48, 24, platform1)

static BEGIN_ANIM(platform1_idle)
ANIM_FRAME(platform1_idle_0)
ANIM_FRAME(platform1_idle_1)
ANIM_FRAME(platform1_idle_2)
ANIM_FRAME(platform1_idle_3)
ANIM_FRAME(platform1_idle_4)
ANIM_FRAME(platform1_idle_5)
ANIM_FRAME(platform1_idle_6)
ANIM_FRAME(platform1_idle_7)
END_ANIM

static DEFANIM(platform1_idle)

static BBOX(cloud1, 0, 4, 48, 12)

static FRAMEB(cloud1_idle_0, bmp_cloud1, 0, 0, 48, 16, cloud1)
static FRAMEB(cloud1_idle_1, bmp_cloud1, 0, 16, 48, 16, cloud1)
static FRAMEB(cloud1_idle_2, bmp_cloud1, 0, 32, 48, 16, cloud1)
static FRAMEB(cloud1_idle_3, bmp_cloud1, 0, 48, 48, 16, cloud1)

static BEGIN_ANIM(cloud1_idle)
ANIM_FRAME(cloud1_idle_0)
ANIM_FRAME(cloud1_idle_1)
ANIM_FRAME(cloud1_idle_2)
ANIM_FRAME(cloud1_idle_3)
END_ANIM

static DEFANIM(cloud1_idle)

static BBOX(cloud1_alt1, 0, 4, 48, 12)

static FRAMEB(cloud1_alt1_idle_0, bmp_cloud1, 48, 0, 48, 16, cloud1_alt1)
static FRAMEB(cloud1_alt1_idle_1, bmp_cloud1, 48, 16, 48, 16, cloud1_alt1)
static FRAMEB(cloud1_alt1_idle_2, bmp_cloud1, 48, 32, 48, 16, cloud1_alt1)
static FRAMEB(cloud1_alt1_idle_3, bmp_cloud1, 48, 48, 48, 16, cloud1_alt1)

static BEGIN_ANIM(cloud1_alt1_idle)
ANIM_FRAME(cloud1_alt1_idle_0)
ANIM_FRAME(cloud1_alt1_idle_1)
ANIM_FRAME(cloud1_alt1_idle_2)
ANIM_FRAME(cloud1_alt1_idle_3)
END_ANIM

static DEFANIM(cloud1_alt1_idle)

int how_far_y_platforms(int x0, int y0, int x1, int y1, int delta)
{
	struct actor *pplat;
	struct rect bbox;
	int i;

	if (delta <= 0)
		return delta;

	for (i = AC_PLATFORM_0; i < AC_PLATFORM_LAST + 1; i++) {
		pplat = get_actor(i);
		if (is_free_actor(pplat))
			continue;

		if (pplat->psp == NULL || pplat->psp->pframe == NULL ||
			pplat->psp->pframe->pbbox == NULL)
		{
			continue;
		}

		get_actor_bbox(&bbox, pplat);
		if (x1 < bbox.x || x0 > bbox.x + bbox.w)
			continue;
		if (y1 <= bbox.y && y1 + delta >= bbox.y)
			return bbox.y - y1;
	}

	return delta;
}

static void platform_idle(struct actor *pac)
{
	int oldx, move_cosmo;
	struct actor *pcosmo;
	struct rect bbox, cosmobox;

	/* move the cosmonaut if above us */
	move_cosmo = 1;
	pcosmo = get_actor(AC_COSMONAUT);
	if (pcosmo->update != cosmonaut_idle &&
		pcosmo->update != cosmonaut_walk &&
		pcosmo->update != cosmonaut_down &&
		pcosmo->update != cosmonaut_die)
	{
		move_cosmo = 0;
	}

	if (move_cosmo && 
		(pcosmo->psp == NULL || pcosmo->psp->pframe == NULL ||
		 pcosmo->psp->pframe->pbbox == NULL))
	{
		move_cosmo = 0;
	}

	if (move_cosmo) {
		get_actor_bbox(&bbox, pac);
		get_actor_bbox(&cosmobox, pcosmo);
		if (!(cosmobox.x + cosmobox.w - 1 >= bbox.x &&
			cosmobox.x <= bbox.x + bbox.w &&
			cosmobox.y + cosmobox.h -1 == bbox.y))
		{
			move_cosmo = 0;
		}
	}

	oldx = pac->psp->x;
	update_actor_path(pac, 0);
	if (move_cosmo) {
		pcosmo->psp->x += cosmonaut_how_far_x(pcosmo,
			pac->psp->x - oldx, NULL);
	}
}

static struct actor *spawn_platform(int tx, int ty, struct anim *panim,
	int anim_speed)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_PLATFORM_0, SP_PLATFORM_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, panim, anim_speed, 1);
	if (psp->pframe->pbbox != NULL) {
		psp->y -= te_p2w(psp->pframe->pbbox->y);
	}

	pac = get_free_actor(AC_PLATFORM_0, AC_PLATFORM_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = platform_idle;

	return pac;
}

static struct actor *spawn_platform_fp(int tx, int ty)
{
	int r, anim, anim_speed;
	struct anim *panim;

	r = tokscanf(NULL, "ii", &anim, &anim_speed);
	if (r != 2) {
		return NULL;
	}

	switch (anim) {
	default:
	case 0: panim = &am_platform1_idle; break;
	case 1: panim = &am_cloud1_idle; break;
	case 2: panim = &am_cloud1_alt1_idle; break;
	}

	return spawn_platform(tx, ty, panim, anim_speed);
}

void platform_init(void)
{
	register_bitmap(&bmp_platfor1, "platfor1", 1, 0);
	register_bitmap(&bmp_cloud1, "cloud1", 1, 0);
	register_spawn_fn("platform", spawn_platform_fp);
}
