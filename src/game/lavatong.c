/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "lavatong.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "cosmonau.h"
#include "debug.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"

static struct bmp *bmp_lavatong = NULL;
static struct wav *wav_lavatong = NULL;

static BBOX(lavatongue_0, 4, 39, 3, 9)
static BBOX(lavatongue_1, 4, 30, 4, 18)
static BBOX(lavatongue_2, 5, 12, 8, 36)
static BBOX(lavatongue_3, 7, 8, 17, 37)
static BBOX(lavatongue_4, 9, 7, 16, 36)
static BBOX(lavatongue_5, 13, 8, 13, 40)
static BBOX(lavatongue_6, 23, 31, 4, 17)
static BBOX(lavatongue_7, 23, 37, 3, 11)

static FRAMEB(lavatongue_jump_0, bmp_lavatong, 0, 0, 32, 48, lavatongue_0);
static FRAMEB(lavatongue_jump_1, bmp_lavatong, 32, 0, 32, 48, lavatongue_1);
static FRAMEB(lavatongue_jump_2, bmp_lavatong, 64, 0, 32, 48, lavatongue_2);
static FRAMEB(lavatongue_jump_3, bmp_lavatong, 96, 0, 32, 48, lavatongue_3);
static FRAMEB(lavatongue_jump_4, bmp_lavatong, 128, 0, 32, 48, lavatongue_4);
static FRAMEB(lavatongue_jump_5, bmp_lavatong, 160, 0, 32, 48, lavatongue_5);
static FRAMEB(lavatongue_jump_6, bmp_lavatong, 192, 0, 32, 48, lavatongue_6);
static FRAMEB(lavatongue_jump_7, bmp_lavatong, 224, 0, 32, 48, lavatongue_7);

static BEGIN_ANIM(lavatongue_jump)
ANIM_FRAME(lavatongue_jump_0)
ANIM_FRAME(lavatongue_jump_1)
ANIM_FRAME(lavatongue_jump_2)
ANIM_FRAME(lavatongue_jump_3)
ANIM_FRAME(lavatongue_jump_4)
ANIM_FRAME(lavatongue_jump_5)
ANIM_FRAME(lavatongue_jump_6)
ANIM_FRAME(lavatongue_jump_7)
END_ANIM

static DEFANIM(lavatongue_jump)

#define ACTIONS_INDEX i0
#define ACTION_INDEX  i1
#define TX vx
#define TY vy

enum {
	NENTS = AC_ENEMY_LAST - AC_ENEMY_0 + 1,
	NACTS = 16,
};


static char s_actions[NENTS][NACTS];

static void exec_next_action(struct actor *pac);
static void set_idle(struct actor *pac);

static void reset_lavatongues(int mapid)
{
	int i;

	for (i = 0; i < NENTS; i++)
		s_actions[i][0] = '\0';
}

/* We wait for the animation to end and then for the time to complete. */
static void lavatongue_jump(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	pac->t = dectime(pac->t);
	if (pac->psp != NULL && !te_is_anim_playing(pac->psp)) {
		te_free_sprite(pac->psp);
		pac->psp = NULL;
	}

	if (pac->t <= 0) {
		/* Enable this assert to check that time is fully
		 * synchronized. */
		if (DEBUG_ON) {
			kasserta(pac->psp == NULL);
		}
		exec_next_action(pac);
	}
}

static void set_jump(struct actor *pac, int dir)
{
	struct sprite *psp;

	if (kassert_fails(dir == LDIR || dir == RDIR))
		dir = LDIR;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (psp == NULL) {
		set_idle(pac);
		return;
	}

	mixer_play(wav_lavatong);
	psp->x = pac->TX * TE_VBTW;
	psp->y = pac->TY * TE_VBTW; 
	te_set_anim(psp, &am_lavatongue_jump, 3, 0);
	pac->psp = psp;
	set_actor_dir(pac, dir);
	pac->t = FPS_FULL;
	pac->update = lavatongue_jump;
}

static void lavatongue_idle(struct actor *pac)
{
	pac->t = dectime(pac->t);
	if (pac->t <= 0)
		exec_next_action(pac);
}

static void set_idle(struct actor *pac)
{
	pac->t = FPS_FULL;
	pac->update = lavatongue_idle;
}

static void lavatongue_void(struct actor *pac)
{
}

static void exec_next_action(struct actor *pac)
{
	char *actions;
	char c;

	if (pac->ACTIONS_INDEX < 0)
		return;

	if (pac->psp != NULL) {
		te_free_sprite(pac->psp);
		pac->psp = NULL;
	}

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
	case 'L':
		set_jump(pac, LDIR);
		break;
	case 'R':
		set_jump(pac, RDIR);
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

	if(kassert_fails(i != NENTS)) {
		pac->ACTIONS_INDEX = -1;
	}
}

static struct actor *spawn_lavatongue(int tx, int ty, const char *actions)
{
	struct actor *pac;

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->TX = tx;
	pac->TY = ty;
	pac->update = lavatongue_void;
	store_actions(pac, actions);
	pac->ACTION_INDEX = 0;
	exec_next_action(pac);

	return pac;
}

static struct actor *spawn_lavatongue_fp(int tx, int ty)
{
	int r;
	char actions[NACTS];

	r = tokscanf(NULL, "s", actions, NELEMS(actions));
	if (r != 1) {
		actions[0] = 'S';
		actions[1] = '\0';
	}

	return spawn_lavatongue(tx, ty, actions);
}

void lavatong_init(void)
{
	register_bitmap(&bmp_lavatong, "lavatong", 1, 0);
	register_spawn_fn("lavatongue", spawn_lavatongue_fp);
	register_map_init_fn(reset_lavatongues);
	register_sound(&wav_lavatong, "lavatong");
}
