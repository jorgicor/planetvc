/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "draco.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "cosmonau.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <string.h>

static struct bmp *bmp_draco = NULL;
static struct bmp *bmp_firedrac = NULL;
static struct wav *wav_draco; 

static BBOX(draco, 8, 21, 17, 11)

static FRAMEB(draco_idle_0, bmp_draco, 0, 0, 32, 48, draco)
static FRAMEB(draco_shoot_0, bmp_draco, 32, 0, 32, 48, draco)
static FRAMEB(draco_shoot_1, bmp_draco, 64, 0, 32, 48, draco)

static BEGIN_ANIM(draco_idle)
ANIM_FRAME(draco_idle_0)
END_ANIM

static BEGIN_ANIM(draco_shoot)
ANIM_FRAME(draco_shoot_0)
ANIM_FRAME(draco_shoot_1)
END_ANIM

static BEGIN_ANIM(draco_end_shoot)
ANIM_FRAME(draco_shoot_1)
ANIM_FRAME(draco_shoot_0)
ANIM_FRAME(draco_idle_0)
END_ANIM

static DEFANIM(draco_idle)
static DEFANIM(draco_shoot)
static DEFANIM(draco_end_shoot)

static BBOX(dracofire_0, 9, 6, 3, 2)
static BBOX(dracofire_1, 9, 6, 5, 3)
static BBOX(dracofire_2, 10, 6, 15, 5)
static BBOX(dracofire_3, 10, 6, 32, 5)
static BBOX(dracofire_4, 11, 4, 48, 6)
static BBOX(dracofire_5, 11, 4, 48, 6)
static BBOX(dracofire_6, 15, 4, 44, 6)
static BBOX(dracofire_7, 39, 4, 26, 7)
static BBOX(dracofire_8, 43, 4, 11, 5)

static FRAMEB(dracofire_start_0, bmp_firedrac, 0, 0, 64, 16, dracofire_0)
static FRAMEB(dracofire_start_1, bmp_firedrac, 0, 16, 64, 16, dracofire_1)
static FRAMEB(dracofire_start_2, bmp_firedrac, 0, 32, 64, 16, dracofire_2)
static FRAMEB(dracofire_start_3, bmp_firedrac, 0, 48, 64, 16, dracofire_3)
static FRAMEB(dracofire_loop_0, bmp_firedrac, 0, 64, 64, 16, dracofire_4)
static FRAMEB(dracofire_loop_1, bmp_firedrac, 0, 80, 64, 16, dracofire_5)
static FRAMEB(dracofire_end_0, bmp_firedrac, 0, 96, 64, 16, dracofire_6)
static FRAMEB(dracofire_end_1, bmp_firedrac, 0, 112, 64, 16, dracofire_7)
static FRAMEB(dracofire_end_2, bmp_firedrac, 0, 128, 64, 16, dracofire_8)

static BEGIN_ANIM(dracofire_start)
ANIM_FRAME(dracofire_start_0)
ANIM_FRAME(dracofire_start_1)
ANIM_FRAME(dracofire_start_2)
ANIM_FRAME(dracofire_start_3)
END_ANIM

static BEGIN_ANIM(dracofire_loop)
ANIM_FRAME(dracofire_loop_0)
ANIM_FRAME(dracofire_loop_1)
END_ANIM

static BEGIN_ANIM(dracofire_end)
ANIM_FRAME(dracofire_end_0)
ANIM_FRAME(dracofire_end_1)
ANIM_FRAME(dracofire_end_2)
END_ANIM

static DEFANIM(dracofire_start)
static DEFANIM(dracofire_loop)
static DEFANIM(dracofire_end)

#define ACTIONS_INDEX	i0
#define ACTION_INDEX	i1
#define PDRACO		pac2

enum {
	NENTS = AC_ENEMY_LAST - AC_ENEMY_0 + 1,
	NACTS = 16,
};


static char s_actions[NENTS][NACTS];

static void exec_next_action(struct actor *pac);

/* fire ending animation */
static void dracofire_end(struct actor *pac)
{
	struct actor *pdraco;

	pac->t = dectime(pac->t);
	if (!te_is_anim_playing(pac->psp)) {
		/* Enable this assert to check that dragons are fully
		 * synchronized. */
		/* This can fail if we press ESC to show the in game menu.
		 * tilengin will update the animations but our update is not
		 * called, so this is not synchronized.
		 * Don't enable in release version!!!
		 */
		if (PP_DEBUG) {
			kasserta(pac->t == 0);
		}
		pdraco = pac->PDRACO;
		te_free_sprite(pac->psp);
		free_actor(pac);
		exec_next_action(pdraco);
	}
}

/* fire loop aimation */
static void dracofire_loop(struct actor *pac)
{
	int n;

	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	pac->t = dectime(pac->t);
	n = am_dracofire_end.nframes * 4 * FPS_MUL;
	if (pac->t <= n) {
		te_set_anim(pac->PDRACO->psp, &am_draco_end_shoot, 4, 0);
		te_set_anim(pac->psp, &am_dracofire_end, 4, 0);
		pac->update = dracofire_end;
	}
}

/* fire start animation */
static void dracofire_start(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	pac->t = dectime(pac->t);
	if (!te_is_anim_playing(pac->psp)) {
		te_set_anim(pac->psp, &am_dracofire_loop, 4, 1);
		pac->update = dracofire_loop;
	}
}

static struct actor *spawn_dracofire(struct actor *pdraco, int time)
{
	struct actor *pac;
	struct sprite *psp;

	psp = te_get_free_sprite(SP_ESHOT_0, SP_ESHOT_LAST + 1);
	if (psp == NULL) {
		return NULL;
	}

	pac = get_free_actor(AC_ESHOT_0, AC_ESHOT_LAST + 1);
	if (pac == NULL) {
		te_free_sprite(psp);
		return NULL;
	}

	psp->x = pdraco->psp->x;
	psp->y = pdraco->psp->y + TE_VBTW; 
	if (pdraco->dir == RDIR) {
		psp->x += TE_VBTW;
	} else {
		psp->x -= TE_VBTW * 3;
	}
	te_set_anim(psp, &am_dracofire_start, 4, 0);
	pac->psp = psp;
	pac->t = time;
	pac->update = dracofire_start;
	pac->PDRACO = pdraco;
	set_actor_dir(pac, pac->PDRACO->dir);
	return pac;
}

/* spitting fire */
static void draco_fire(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);
}

static void draco_start_fire(struct actor *pac)
{
	struct actor *pfire;
	int ot;

	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	ot = pac->t;
	pac->t = dectime(pac->t);
	if (!te_is_anim_playing(pac->psp)) {
		pfire = spawn_dracofire(pac, ot);
		if (pfire == NULL) {
			exec_next_action(pac);
		} else {
			pac->update = draco_fire;
		}
	}
}

static void set_start_fire(struct actor *pac, char c)
{
	pac->t = FPS_FULL;
	switch (c) {
	case 'B': pac->t *= 3; break;
	case 'C': pac->t *= 4; break;
	default: pac->t *= 2; break;
	}

	mixer_play(wav_draco);
	te_set_anim(pac->psp, &am_draco_shoot, 4, 0);
	pac->update = draco_start_fire;
}

static void draco_idle(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	pac->t = dectime(pac->t);
	if (pac->t <= 0)
		exec_next_action(pac);
}

static void set_idle(struct actor *pac)
{
	pac->t = FPS_FULL;
	te_set_anim(pac->psp, &am_draco_idle, 4, 1);
	pac->update = draco_idle;
}

static void draco_void(struct actor *pac)
{
}

static void exec_next_action(struct actor *pac)
{
	char *actions;
	char c;

	if (pac->ACTIONS_INDEX < 0)
		return;

	actions = s_actions[pac->ACTIONS_INDEX];
	c = actions[pac->ACTION_INDEX++];
	if (c == '\0') {
		c = actions[0];
		pac->ACTION_INDEX = 1;
	}

	switch (c) {
	default:
	case 'S':
		set_idle(pac);
		break;
	case 'A':
	case 'B':
	case 'C':
		set_start_fire(pac, c);
		break;
	}
}

static void store_actions(struct actor *pac, const char *actions)
{
	int i, j;

	for (i = 0; i < NENTS; i++) {
		if (s_actions[i][0] == '\0') {
			for (j = 0; j < NACTS - 1 && actions[j] != '\0'; j++)
				s_actions[i][j] = actions[j];
			s_actions[i][j] = '\0';
			pac->ACTIONS_INDEX = i;
			break;
		}
	}

	if (kassert_fails(i != NENTS)) {
		pac->ACTIONS_INDEX = -1;
	}
}

static struct actor *spawn_draco(int tx, int ty, int dir, const char *actions)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_draco_idle, 4, 1);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->psp = psp;
	pac->update = draco_void;
	set_actor_dir(pac, dir);
	store_actions(pac, actions);
	pac->ACTION_INDEX = 0;
	exec_next_action(pac);
	return pac;
}

static struct actor *spawn_draco_fp(int tx, int ty)
{
	int r;
	int idir;
	char dir[5];
	char actions[NACTS];

	r = tokscanf(NULL, "s", dir, NELEMS(dir));
	if (r != 1) {
		dir[0] = '\0';
	}

	idir = RDIR;
	if (strcmp(dir, "ldir") == 0) {
		idir = LDIR;
	} else if (strcmp(dir, "rdir") == 0) {
		idir = RDIR;
	}

	r = tokscanf(NULL, "s", actions, NELEMS(actions));
	if (r != 1) {
		actions[0] = 'S';
		actions[1] = '\0';
	}

	return spawn_draco(tx, ty, idir, actions);
}
	
static void reset_dracos(int mapid)
{
	int i;

	for (i = 0; i < NENTS; i++)
		s_actions[i][0] = '\0';
}

void draco_init(void)
{
	register_bitmap(&bmp_draco, "draco", 1, 128);
	register_bitmap(&bmp_firedrac, "firedrac", 1, 128);
	register_map_init_fn(reset_dracos);
	register_spawn_fn("draco", spawn_draco_fp);
	register_sound(&wav_draco, "draco");
}
