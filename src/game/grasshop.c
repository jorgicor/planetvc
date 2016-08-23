#include "grasshop.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cosmonau.h"
#include "sounds.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"

static struct bmp *bmp_grasshop = NULL;
static struct wav *wav_grshopjp = NULL;

static BBOX(grasshopper_0, 4, 22, 21, 9)
static BBOX(grasshopper_1, 6, 17, 19, 10)
static BBOX(grasshopper_2, 5, 16, 21, 9)
static BBOX(grasshopper_3, 10, 8, 18, 10)

static FRAMEB(grasshopper_jump_0, bmp_grasshop, 0, 0, 32, 32, grasshopper_0)
static FRAMEB(grasshopper_jump_1, bmp_grasshop, 32, 0, 32, 32, grasshopper_1)
static FRAMEB(grasshopper_jump_2, bmp_grasshop, 64, 0, 32, 32, grasshopper_2)
static FRAMEB(grasshopper_jump_3, bmp_grasshop, 96, 0, 32, 32, grasshopper_3)

static BEGIN_ANIM(grasshopper_jump)
ANIM_FRAME(grasshopper_jump_0)
ANIM_FRAME(grasshopper_jump_1)
ANIM_FRAME(grasshopper_jump_2)
ANIM_FRAME(grasshopper_jump_3)
END_ANIM

static BEGIN_ANIM(grasshopper_alight)
ANIM_FRAME(grasshopper_jump_3)
ANIM_FRAME(grasshopper_jump_2)
ANIM_FRAME(grasshopper_jump_1)
ANIM_FRAME(grasshopper_jump_0)
END_ANIM

static DEFANIM(grasshopper_jump)
static DEFANIM(grasshopper_alight)

#define ACTIONS_INDEX i0
#define ACTION_INDEX  i1

enum {
	VX = TE_ONE,
	NENTS = AC_ENEMY_LAST - AC_ENEMY_0 + 1,
	NACTS = 16,
};


static char s_actions[NENTS][NACTS];

static void exec_next_action(struct actor *pac);

static void reset_grasshoppers(int mapid)
{
	int i;

	for (i = 0; i < NENTS; i++)
		s_actions[i][0] = '\0';
}

static void grasshopper_alight(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	if (!te_is_anim_playing(pac->psp))
		exec_next_action(pac);
}

static void set_alight(struct actor *pac)
{
	te_set_anim(pac->psp, &am_grasshopper_alight, 1, 0);
	pac->update = grasshopper_alight;
}

static void grasshopper_jump(struct actor *pac)
{
	int oy, od, d;
	struct sprite *psp;
	int fw, fh;

	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	psp = pac->psp;
	move_actor_x(pac);
	oy = psp->y;
	move_actor_y(pac);
	od = psp->y - oy;
	psp->y = oy;

	fw = te_p2w(psp->pframe->r.w);
	fh = te_p2w(psp->pframe->r.h);

	d = how_far_y(psp->x, psp->y, psp->x + fw - 1, psp->y + fh - 1, od);
	psp->y += d;

	if (psp->y == oy && how_far_y(psp->x, psp->y, psp->x + fw - 1,
				      psp->y + fh - 1, 1) == 0)
	{
		set_alight(pac);
	}
}

static void set_jump(struct actor *pac)
{
	pac->ax = 0;
	pac->ay = 16;
	pac->vx *= VX;
	pac->vy = -(TE_ONE / 2) * 5;
	pac->update = grasshopper_jump;
}

static void grasshopper_start_jump(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	if (!te_is_anim_playing(pac->psp))
		set_jump(pac);
}

static void set_start_jump(struct actor *pac, int to)
{
	mixer_play(wav_grshopjp);
	if (to != 0) {
		set_actor_dir(pac, (to > 0) ? RDIR : LDIR);
	}
	te_set_anim(pac->psp, &am_grasshopper_jump, 1, 0);
	pac->vx = to;
	pac->update = grasshopper_start_jump;
}

static void grasshopper_idle(struct actor *pac)
{
	if (actor_overlaps(get_actor(AC_COSMONAUT), pac))
		cosmonaut_set_dying(1);

	pac->t = dectime(pac->t);
	if (pac->t <= 0)
		exec_next_action(pac);
}

static void set_idle(struct actor *pac)
{
	pac->t = FPS_FULL * 3;
	pac->update = grasshopper_idle;
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
	case 'U':
		set_start_jump(pac, 0);
		break;
	case 'L':
		set_start_jump(pac, -1);
		break;
	case 'R':
		set_start_jump(pac, 1);
		break;
	}
}

static void grasshopper_void(struct actor *pac)
{
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

static struct actor *spawn_grasshopper(int tx, int ty, const char *actions)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_free_sprite(SP_ENEMY_0, SP_ENEMY_LAST + 1);
	if (kassert_fails(psp != NULL))
		return NULL;

	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW; 
	te_set_anim(psp, &am_grasshopper_jump, 0, 0);

	pac = get_free_actor(AC_ENEMY_0, AC_ENEMY_LAST + 1);
	if (kassert_fails(pac != NULL))
		return NULL;

	pac->update = grasshopper_void;
	pac->psp = psp;
	store_actions(pac, actions);
	pac->ACTION_INDEX = 0;
	exec_next_action(pac);

	return pac;
}

static struct actor *spawn_grasshopper_fp(int tx, int ty)
{
	int r;
	char actions[NACTS];

	r = tokscanf(NULL, "s", actions, NELEMS(actions));
	if (r != 1) {
		actions[0] = 'S';
		actions[1] = '\0';
	}

	return spawn_grasshopper(tx, ty, actions);
}

void grasshop_init(void)
{
	register_bitmap(&bmp_grasshop, "grasshop", 1, 128);
	register_spawn_fn("grasshopper", spawn_grasshopper_fp);
	register_map_init_fn(reset_grasshoppers);
	register_sound(&wav_grshopjp, "grshopjp");
}
