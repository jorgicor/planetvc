/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "spider.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "debug.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include "config.h"
#include <string.h>

#ifdef HAVE_TGMATH_H
#	include <tgmath.h>
#else
#	include <math.h>
#endif

static struct bmp *bmp_spider = NULL;

static BBOX(spider, 11, 2, 10, 17)

static FRAMEB(spider_idle_0, bmp_spider, 0, 0, 32, 32, spider)
static FRAMEB(spider_idle_1, bmp_spider, 32, 0, 32, 32, spider)
static FRAMEB(spider_idle_2, bmp_spider, 64, 0, 32, 32, spider)

static BEGIN_ANIM(spider_idle)
ANIM_FRAME(spider_idle_0)
ANIM_FRAME(spider_idle_1)
ANIM_FRAME(spider_idle_2)
END_ANIM

static DEFANIM(spider_idle)

#define PI 3.2416f

#define RADIUS		t
#define ANGLE		i0
#define START_ANGLE	i1
#define W		ax
#define AW		ay

enum {
	SPIDER_NLINES = AC_ENEMY_LAST - AC_ENEMY_0 + 1
};

static const int s_spider_master_line[] = {
	TE_SHAPEC_COLOR, (int) 0x00800080,
	TE_SHAPEC_REL_MOVE, 16, 0,
	TE_SHAPEC_LINETO, 0, 0,
	TE_SHAPEC_END
};

static int s_spider_lines[SPIDER_NLINES][NELEMS(s_spider_master_line)];
static int s_spider_used_lines[SPIDER_NLINES];

static void reset_spiders(int mapid)
{
	memset(s_spider_used_lines, 0, sizeof(s_spider_used_lines));
}

static void spider_set_pos(struct actor *pac)
{
	float rads;

	rads = (pac->ANGLE * 1.f / TE_ONE) * PI / 180; 
	pac->psp->x = pac->vx - te_p2w(pac->psp->pframe->r.w / 2);
	pac->psp->x += (int) (cos(rads) * pac->RADIUS);
	pac->psp->y = pac->vy + (int) (sin(rads) * pac->RADIUS);
}

static void spider_set_frame(struct actor *pac)
{
	if (pac->W > 0) {
		te_set_anim_frame(pac->psp, 0);
	} else {
		te_set_anim_frame(pac->psp, 2);
	}
}

static void spider_idle(struct actor *pac)
{
	int oldw, oldangle, mid;

	oldangle = pac->ANGLE;
	oldw = pac->W;
	pac->W += pac->AW;
	pac->ANGLE += asr(oldw + pac->W, 1); 
	mid = 90 * TE_ONE;

	if (oldangle < mid && pac->ANGLE >= mid) {
		pac->ANGLE = mid;
		pac->AW = -pac->AW;
	} else if (oldangle > mid && pac->ANGLE <= mid) {
		pac->ANGLE = mid;
		pac->AW = -pac->AW;
	}

	spider_set_pos(pac);
	spider_set_frame(pac);

	if (actor_overlaps(get_actor(AC_COSMONAUT), pac)) {
		cosmonaut_set_dying(1);
	}
}

static void spider_assign_line(struct actor *pac)
{
	int linei, shi;
	struct shape *psh;

	for (linei = 0; linei < SPIDER_NLINES; linei++) {
		if (s_spider_used_lines[linei] == 0) {
			break;
		}
	}
	if (linei >= SPIDER_NLINES) {
		return;
	}
	shi = te_get_free_shape(1, TE_NSHAPES);
	if (shi == 0) {
		return;
	}

	s_spider_used_lines[linei] = 1;
	pac->psp->shapei = shi;
	psh = te_get_shape(shi);
	memcpy(&s_spider_lines[linei], s_spider_master_line,
		sizeof(s_spider_master_line));  
	psh->code = s_spider_lines[linei]; 
	psh->code[6] = te_w2p(pac->vx); 
	psh->code[7] = te_w2p(pac->vy);
}

/*
 * radius is in pixels.
 * aw is angular velocity. 256 is 1 degree.
 * start_angle is the start angle between 1 and 89.
 */
static struct actor *spawn_spider(int tx, int ty, int radius, int aw,
				  int start_angle)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = (tx * TE_VBTW) + (TE_VBTW / 2);
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_spider_idle, 0, 0);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = spider_idle;

	if (start_angle < 1)
		start_angle = 1;
	else if (start_angle > 89)
		start_angle = 89;

	pac->vx = psp->x;
	pac->vy = psp->y;
	pac->ANGLE = start_angle * TE_ONE;
	pac->W = 0;
	pac->AW = aw;
	pac->RADIUS = te_p2w(radius);

	spider_set_pos(pac);
	spider_set_frame(pac);
	spider_assign_line(pac);

	return pac;
}

static struct actor *spawn_spider_fp(int tx, int ty)
{
	int r;
	int radius, aw, start_angle;

	r = tokscanf(NULL, "iii", &radius, &aw, &start_angle);
	if (r != 3) {
		return NULL;
	}

	return spawn_spider(tx, ty, radius, aw, start_angle);
}

void spider_init(void)
{
	if (DEMO_ON)
		return;
	register_bitmap(&bmp_spider, "spider", 1, 128);
	register_spawn_fn("spider", spawn_spider_fp);
	register_map_init_fn(reset_spiders);
}
