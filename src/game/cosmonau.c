#include "cosmonau.h"
#include "game.h"
#include "input.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "sounds.h"
#include "oxigen.h"
#include "balloon.h"
#include "platform.h"
#include "stargate.h"
#include "hud.h"
#include "demo_st.h"
#include "gplay_st.h"
#include "initfile.h"
#include "gamelib/mixer.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include "kernel/kernel.h"

static struct bmp *bmp_cosmonau = NULL;
static struct bmp *bmp_diewater = NULL;
static struct wav *wav_jump = NULL;
static struct wav *wav_teleport = NULL;
static struct wav *wav_diewater = NULL;
static struct wav *wav_dielava = NULL;
static struct wav *wav_hit = NULL;
/* static struct wav *wav_floorhit = NULL; */

static BBOX(cosmonaut, 10, 2, 12, 30)
static BBOX(cosmonaut_down, 10, 10, 12, 22)

static FRAMEB(cosmonaut_walkr_0, bmp_cosmonau, 0, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_1, bmp_cosmonau, 32, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_2, bmp_cosmonau, 64, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_3, bmp_cosmonau, 96, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_4, bmp_cosmonau, 128, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_5, bmp_cosmonau, 160, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_6, bmp_cosmonau, 192, 0, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_walkr_7, bmp_cosmonau, 224, 0, 32, 32, cosmonaut)

static FRAMEB(cosmonaut_idler_0, bmp_cosmonau, 0, 32, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_idler_1, bmp_cosmonau, 32, 32, 32, 32, cosmonaut)

static FRAMEB(cosmonaut_downr_0, bmp_cosmonau, 64, 32, 32, 32, cosmonaut_down)

static FRAMEB(cosmonaut_jumpr_0, bmp_cosmonau, 96, 32, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_jumpr_1, bmp_cosmonau, 128, 32, 32, 32, cosmonaut)

static FRAME(cosmonaut_dielava_7, bmp_cosmonau, 160, 32, 32, 32)
static FRAME(cosmonaut_dielava_8, bmp_cosmonau, 192, 32, 32, 32)
static FRAME(cosmonaut_dielava_9, bmp_cosmonau, 224, 32, 32, 32)

static FRAME(cosmonaut_dielava_0, bmp_cosmonau, 0, 64, 32, 32)
static FRAME(cosmonaut_dielava_1, bmp_cosmonau, 32, 64, 32, 32)
static FRAME(cosmonaut_dielava_2, bmp_cosmonau, 64, 64, 32, 32)
static FRAME(cosmonaut_dielava_3, bmp_cosmonau, 96, 64, 32, 32)
static FRAME(cosmonaut_dielava_4, bmp_cosmonau, 128, 64, 32, 32)
static FRAME(cosmonaut_dielava_5, bmp_cosmonau, 160, 64, 32, 32)
static FRAME(cosmonaut_dielava_6, bmp_cosmonau, 192, 64, 32, 32)

static FRAMEB(cosmonaut_diejump_0, bmp_cosmonau, 224, 64, 32, 32, cosmonaut)

static FRAMEB(cosmonaut_die_0, bmp_cosmonau, 0, 96, 32, 32, cosmonaut_down)
static FRAMEB(cosmonaut_die_1, bmp_cosmonau, 32, 96, 32, 32, cosmonaut_down)
static FRAMEB(cosmonaut_die_2, bmp_cosmonau, 64, 96, 32, 32, cosmonaut_down)
static FRAMEB(cosmonaut_die_3, bmp_cosmonau, 96, 96, 32, 32, cosmonaut_down)
static FRAMEB(cosmonaut_die_4, bmp_cosmonau, 128, 96, 32, 32, cosmonaut_down)

static FRAMEB(cosmonaut_teleport_0, bmp_cosmonau, 160, 96, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_teleport_1, bmp_cosmonau, 192, 96, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_teleport_2, bmp_cosmonau, 224, 96, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_teleport_3, bmp_cosmonau, 0, 128, 32, 32, cosmonaut)
static FRAMEB(cosmonaut_teleport_4, bmp_cosmonau, 32, 128, 32, 32, cosmonaut)

static BEGIN_ANIM(cosmonaut_walk)
ANIM_FRAME(cosmonaut_walkr_6)
ANIM_FRAME(cosmonaut_walkr_7)
ANIM_FRAME(cosmonaut_walkr_0)
ANIM_FRAME(cosmonaut_walkr_1)
ANIM_FRAME(cosmonaut_walkr_2)
ANIM_FRAME(cosmonaut_walkr_3)
ANIM_FRAME(cosmonaut_walkr_4)
ANIM_FRAME(cosmonaut_walkr_5)
END_ANIM

static DEFANIM(cosmonaut_walk)

static BEGIN_ANIM(cosmonaut_idle)
ANIM_FRAME(cosmonaut_idler_0)
ANIM_FRAME(cosmonaut_idler_1)
END_ANIM

static DEFANIM(cosmonaut_idle)

static BEGIN_ANIM(cosmonaut_down)
ANIM_FRAME(cosmonaut_downr_0)
END_ANIM

static DEFANIM(cosmonaut_down)

static BEGIN_ANIM(cosmonaut_jump)
ANIM_FRAME(cosmonaut_jumpr_0)
ANIM_FRAME(cosmonaut_jumpr_1)
END_ANIM

static DEFANIM(cosmonaut_jump)

static BEGIN_ANIM(cosmonaut_dielava_start)
ANIM_FRAME(cosmonaut_dielava_0)
ANIM_FRAME(cosmonaut_dielava_1)
ANIM_FRAME(cosmonaut_dielava_2)
ANIM_FRAME(cosmonaut_dielava_3)
ANIM_FRAME(cosmonaut_dielava_2)
ANIM_FRAME(cosmonaut_dielava_3)
ANIM_FRAME(cosmonaut_dielava_2)
ANIM_FRAME(cosmonaut_dielava_3)
ANIM_FRAME(cosmonaut_dielava_4)
ANIM_FRAME(cosmonaut_dielava_5)
ANIM_FRAME(cosmonaut_dielava_6)
END_ANIM

static DEFANIM(cosmonaut_dielava_start)

static BEGIN_ANIM(cosmonaut_dielava_end)
ANIM_FRAME(cosmonaut_dielava_7)
ANIM_FRAME(cosmonaut_dielava_8)
ANIM_FRAME(cosmonaut_dielava_9)
ANIM_FRAME(cosmonaut_dielava_8)
ANIM_FRAME(cosmonaut_dielava_7)
ANIM_FRAME(cosmonaut_dielava_9)
ANIM_FRAME(cosmonaut_dielava_7)
ANIM_FRAME(cosmonaut_dielava_8)
ANIM_FRAME(cosmonaut_dielava_9)
END_ANIM

static DEFANIM(cosmonaut_dielava_end)

static BEGIN_ANIM(cosmonaut_diejump)
ANIM_FRAME(cosmonaut_diejump_0)
END_ANIM

static DEFANIM(cosmonaut_diejump)

static BEGIN_ANIM(cosmonaut_die)
ANIM_FRAME(cosmonaut_die_0)
ANIM_FRAME(cosmonaut_die_1)
ANIM_FRAME(cosmonaut_die_2)
ANIM_FRAME(cosmonaut_die_3)
END_ANIM

static DEFANIM(cosmonaut_die)

static BEGIN_ANIM(cosmonaut_teleport_a)
ANIM_FRAME(cosmonaut_jumpr_1)
ANIM_FRAME(cosmonaut_teleport_4)
ANIM_FRAME(cosmonaut_teleport_3)
ANIM_FRAME(cosmonaut_teleport_2)
ANIM_FRAME(cosmonaut_teleport_1)
ANIM_FRAME(cosmonaut_teleport_0)
END_ANIM

static BEGIN_ANIM(cosmonaut_teleport_b)
ANIM_FRAME(cosmonaut_teleport_0)
ANIM_FRAME(cosmonaut_teleport_1)
ANIM_FRAME(cosmonaut_teleport_2)
ANIM_FRAME(cosmonaut_teleport_3)
ANIM_FRAME(cosmonaut_teleport_4)
ANIM_FRAME(cosmonaut_jumpr_1)
END_ANIM

static DEFANIM(cosmonaut_teleport_a)
static DEFANIM(cosmonaut_teleport_b)

static FRAME(diewater_0, bmp_diewater, 0, 0, 32, 16)
static FRAME(diewater_1, bmp_diewater, 0, 16, 32, 16)
static FRAME(diewater_2, bmp_diewater, 0, 32, 32, 16)
static FRAME(diewater_3, bmp_diewater, 0, 48, 32, 16)
static FRAME(diewater_4, bmp_diewater, 0, 64, 32, 16)

static BEGIN_ANIM(cosmonaut_diewater)
ANIM_FRAME(diewater_0)
ANIM_FRAME(diewater_1)
ANIM_FRAME(diewater_2)
ANIM_FRAME(diewater_3)
ANIM_FRAME(diewater_4)
END_ANIM

static DEFANIM(cosmonaut_diewater)

static FRAME(ripwater_0, bmp_cosmonau, 64, 128, 32, 32)
static FRAME(ripwater_1, bmp_cosmonau, 96, 128, 32, 32)
static FRAME(ripwater_2, bmp_cosmonau, 128, 128, 32, 32)
static FRAME(ripwater_3, bmp_cosmonau, 160, 128, 32, 32)
static FRAME(ripwater_4, bmp_cosmonau, 192, 128, 32, 32)
static FRAME(ripwater_5, bmp_cosmonau, 224, 128, 32, 32)

static BEGIN_ANIM(cosmonaut_ripwater_start)
ANIM_FRAME(ripwater_0)
ANIM_FRAME(ripwater_1)
ANIM_FRAME(ripwater_2)
END_ANIM

static BEGIN_ANIM(cosmonaut_ripwater_float)
ANIM_FRAME(ripwater_3)
ANIM_FRAME(ripwater_4)
ANIM_FRAME(ripwater_5)
ANIM_FRAME(ripwater_4)
END_ANIM

static DEFANIM(cosmonaut_ripwater_start)
static DEFANIM(cosmonaut_ripwater_float)

enum {
	WALK_V = TE_ONE / 2
};

static void cosmonaut_teleport(struct actor *pac);
static void cosmonaut_jump_y(struct actor *pac);
static void set_idle(struct actor *pac);
static void set_down(struct actor *pac);
static void set_walk(struct actor *pac);
static void set_jump(struct actor *pac, int to);
static void set_fall(struct actor *pac);
static void set_dielava_start(struct actor *pac);
static void set_dielava_end(struct actor *pac);
static void check_lava(struct actor *pac);
static void check_dying(struct actor *pac);

/* Tile positions for cosmonaut start:
 * down-left, down-right, up-left, up-right.
 */
static int s_start_tx[4];
static int s_start_ty[4];

/* Index 0-3 of the default start for the cosmonaut. */
static int s_default_start;

/* If we should die... */
static int s_cosmonaut_should_die;

/* Stargate in which we are appearing or disappearing. */
static struct actor *s_psgate;

/* sound channel for die lava sound loop. */
static int s_dielava_chan = -1;

/* Contains 0, 1, 2 or 3 for each of the player starting positions.
 * It is used to tell the arrow where to point to.
 * It is changed each time we exit from a stargate.
 */
static int s_arrow_dirpos;

static int cosmo_keydown(int key)
{
	if (g_state != &demo_st)
		return is_key_down(key);

	return demo_is_key_down(key);
}

int cosmonaut_how_far_x(struct actor *pac, int d, int *exit)
{
	struct rect box;

	if (exit != NULL) {
		*exit = 0;
	}

	if (kassert_fails(pac != NULL && pac->psp != NULL &&
			  pac->psp->pframe != NULL &&
			  pac->psp->pframe->pbbox != NULL))
	{
		return 0;
	}

	get_actor_bbox(&box, pac);
	d = how_far_x(box.x, box.y, box.x + box.w - 1, box.y + box.h - 1, d);
	if (d < 0) {
		if (box.x + d < 0) {
			d = -box.x;
			if (exit != NULL) {
				*exit = map_has_exit(0);
			}
		}
	} else if (d > 0) {
		if (box.x + box.w - 1 + d > TE_VSCRW) {
			d = TE_VSCRW - (box.x + box.w - 1);
			if (exit != NULL) {
				*exit = map_has_exit(1);
			}
		}
	}

	return d;
}

static int cosmonaut_how_far_y(struct actor *pac, int d)
{
	struct rect box;

	if (kassert_fails(pac != NULL && pac->psp != NULL &&
			  pac->psp->pframe != NULL &&
			  pac->psp->pframe->pbbox != NULL))
	{
		return 0;
	}

	get_actor_bbox(&box, pac);
	d = how_far_y(box.x, box.y, box.x + box.w - 1, box.y + box.h - 1, d);
	d = how_far_y_platforms(box.x, box.y, box.x + box.w -1,
				box.y + box.h -1, d);

	/* Stop on bottom of screen */
	if (d > 0) {
		if (box.y + box.h - 1 + d > TE_VSCRH) {
			d = TE_VSCRH - (box.y + box.h - 1);
		}
	}

	return d;
}

void cosmonaut_dielava_end(struct actor *pac)
{
}

static void set_dielava_end(struct actor *pac)
{
	te_set_anim(pac->psp, &am_cosmonaut_dielava_end, 4, 1);
	pac->update = cosmonaut_dielava_end;
}

void cosmonaut_dielava_start(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		set_dielava_end(pac);
	}
}

static void set_dielava_start(struct actor *pac)
{
	s_dielava_chan = mixer_get_free_channel();
	if (s_dielava_chan >= 0) {
		mixer_queue(s_dielava_chan, wav_dielava, 1);
	}
	cosmonaut_set_dying_ns(1);
	te_set_anim(pac->psp, &am_cosmonaut_dielava_start, 4, 0);
	pac->update = cosmonaut_dielava_start;
}

static void cosmonaut_ripwater_float(struct actor *pac)
{
}

static void set_ripwater_float(struct actor *pac)
{
	te_set_anim(pac->psp, &am_cosmonaut_ripwater_float, 4, 1);
	pac->update = cosmonaut_ripwater_float;
}

static void cosmonaut_ripwater_start(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		set_ripwater_float(pac);
	}
}

static void set_ripwater_start(struct actor *pac)
{
	te_set_anim(pac->psp, &am_cosmonaut_ripwater_start, 4, 0);
	pac->psp->y -= TE_VBTW;
	pac->update = cosmonaut_ripwater_start;
}

static void cosmonaut_diewater(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		set_ripwater_start(pac);
	}
}

static void set_diewater(struct actor *pac)
{
	mixer_play(wav_diewater);
	cosmonaut_set_dying_ns(1);
	set_actor_dir(pac, RDIR);
	te_set_anim(pac->psp, &am_cosmonaut_diewater, 4, 0);
	pac->update = cosmonaut_diewater;
}

static void check_lava(struct actor *pac)
{
	int tx, ty;
	struct rect box;

	if (pac->psp == NULL || pac->psp->pframe == NULL ||
		pac->psp->pframe->pbbox == NULL)
       	{
		return;
	}

	get_actor_bbox(&box, pac);
	tx = te_w2t(box.x + box.w / 2);
	ty = te_w2t(box.y + box.h);
	if ((te_tile_type(te_bg_xy(tx, ty)) & TT_LAVA) != 0) {
		set_dielava_start(pac);
	}
}

static void check_water(struct actor *pac)
{
	int ty;
	struct rect box;

	if (pac->psp == NULL || pac->psp->pframe == NULL ||
		pac->psp->pframe->pbbox == NULL)
       	{
		return;
	}

	get_actor_bbox(&box, pac);
	ty = te_w2t(box.y + box.h);
	if (ty >= TE_BMH) {
		set_diewater(pac);
	}
}

void cosmonaut_die(struct actor *pac)
{
}

static void check_dying(struct actor *pac)
{
	if (!s_cosmonaut_should_die) {
		return;
	}

	if (pac->update != cosmonaut_idle &&
		pac->update != cosmonaut_walk &&
		pac->update != cosmonaut_down)
       	{
		return;
	}

	te_set_anim(pac->psp, &am_cosmonaut_die, 16, 0);
	pac->update = cosmonaut_die;
}

/* Returns 1 if the full movement was done.
 * 'exit' will be -1 if should exit left, 1 if right, 0 if not.
 */
static int cosmonaut_move_x(struct actor *pac, int *exit_side)
{
	int ox, od, d, exit;

	ox = pac->psp->x;
	move_actor_x(pac);
	od = pac->psp->x - ox;
	pac->psp->x = ox;
	d = cosmonaut_how_far_x(pac, od, &exit);
	pac->psp->x += d;
	if (exit_side != NULL) {
		if (g_state == &gplay_st && od > 0 && exit) {
			*exit_side = 1;
		} else if (g_state == &gplay_st && od < 0 && exit) {
			*exit_side = -1;
		} else {
			*exit_side = 0;
		}
	}
	return od == d;
}

/* Returns 1 if the full movement was done. */
static int cosmonaut_move_y(struct actor *pac)
{
	int oy, od, d;

	oy = pac->psp->y;
	move_actor_y(pac);
	od = pac->psp->y - oy;
	pac->psp->y = oy;
	d = cosmonaut_how_far_y(pac, od);
	pac->psp->y += d;
	return od == d;
}

static void cosmonaut_jump_y(struct actor *pac)
{
	int oldy, moved_y;

	oldy = pac->psp->y;
	moved_y = cosmonaut_move_y(pac);
	if (oldy <= pac->psp->y) {
		if (pac->psp->panim == &am_cosmonaut_jump) {
			te_set_anim_frame(pac->psp, 1);
		}
		if (!moved_y && cosmonaut_how_far_y(pac, 1) == 0) {
			/* mixer_play(wav_floorhit); */
			set_idle(pac);
			check_lava(pac);
			check_water(pac);
			check_dying(pac);
		}
	}
}

void cosmonaut_jump(struct actor *pac)
{
	int exit_side;

	if (s_cosmonaut_should_die &&
		pac->psp->panim != &am_cosmonaut_diejump)
	{
		te_set_anim(pac->psp, &am_cosmonaut_diejump, 0, 0);
	}

	if (cosmo_keydown(KLEFT)) {
	       	if (pac->dir != LDIR)
			set_actor_dir(pac, LDIR);
	} else if (cosmo_keydown(KRIGHT)) {
	       	if (pac->dir != RDIR)
			set_actor_dir(pac, RDIR);
	}

	cosmonaut_move_x(pac, &exit_side);
	if (!cosmonaut_is_dead() && !is_training() && s_got_oxigen &&
		exit_side < 0)
       	{
		schedule_map_exit(0);
	} else if (!cosmonaut_is_dead() && !is_training() && s_got_oxigen &&
		exit_side > 0)
       	{
		schedule_map_exit(1);
	} else {
		if (!s_got_oxigen && exit_side != 0) {
			show_balloon(&fr_oxigen_shine_0);
		}
		cosmonaut_jump_y(pac);
	}
}

static void set_jump(struct actor *pac, int to)
{
	struct sprite *psp;

	psp = pac->psp;
	pac->ax = 0;
	pac->ay = 16;
	pac->vx = WALK_V * to;
	pac->vy = -(TE_ONE / 2) * 5;
	te_set_anim(psp, &am_cosmonaut_jump, 0, 0);
	pac->update = cosmonaut_jump;
}

static void set_fall(struct actor *pac)
{
	struct sprite *psp;

	psp = pac->psp;
	pac->ax = 0;
	pac->ay = 16;
	pac->vx = 0;
	pac->vy = 0;
	te_set_anim(psp, &am_cosmonaut_jump, 0, 0);
	te_set_anim_frame(psp, 1);
	pac->update = cosmonaut_jump;
}

/* Checks that we walked from on side to the other of the center
 * vertical line of the main active stargate.
 * Returns the stargate actor or NULL.
 */
static struct actor *check_stargate(struct actor *pac, int oldx)
{
	int i;
	struct actor *psgate;

	for (i = AC_STARGATE_0; i < AC_STARGATE_LAST + 1; i++) {
		psgate = get_actor(i);
		if (psgate->update == NULL)
			continue;

		if (stargate_check_teleport(psgate, pac, oldx))
			return psgate;
	}

	return NULL;
}

static void cosmonaut_teleport(struct actor *pac)
{
}

static void set_teleport(struct actor *pac)
{
	int goto_mapid;

	goto_mapid = stargate_get_goto_mapid(s_psgate);
	if (goto_mapid == initfile_getvar("end_map")) {
		/* win condition */
		schedule_win();
	} else {
		schedule_teleport(stargate_get_goto_mapid(s_psgate),
			  stargate_get_goto_gateid(s_psgate),
			  stargate_get_next_active_gateid(s_psgate));
	}
	pac->update = cosmonaut_teleport;
}

static void cosmonaut_disappear(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		set_teleport(pac);
	}
}

static void set_disappear(struct actor *pac)
{
	te_set_anim(pac->psp, &am_cosmonaut_teleport_a, 3, 0);
	pac->update = cosmonaut_disappear;
}

static void cosmonaut_appear(struct actor *pac)
{
	if (!te_is_anim_playing(pac->psp)) {
		deactivate_stargate(s_psgate);
		set_fall(pac);
	}
}

static void set_appear(struct actor *pac, struct actor *psgate)
{
	int cx, cy;
	struct sprite *psp;

	psp = pac->psp;
	s_psgate = psgate;
	stargate_get_center(psgate, &cx, &cy);
	te_set_anim(psp, &am_cosmonaut_teleport_b, 3, 0);
	psp->x = cx - te_p2w(psp->pframe->r.w / 2);
	psp->y = cy - te_p2w(psp->pframe->r.h / 2);
	pac->update = cosmonaut_appear;
}

static void cosmonaut_ascend(struct actor *pac)
{
	struct sprite *psp;
	int cx, cy, rx, ry;

	psp = pac->psp;
	stargate_get_center(s_psgate, &cx, &cy);
	cx -= te_p2w(psp->pframe->r.w / 2);
	cy -= te_p2w(psp->pframe->r.h / 2);
	line(&rx, &ry, psp->x, psp->y, cx, cy, 256 * FPS_MUL);
	psp->x = rx;
	psp->y = ry;
	if (psp->x == cx && psp->y == cy) {
		set_disappear(pac);
	}
}

static void set_ascend(struct actor *pac, struct actor *psgate)
{
	struct sprite *psp;

	s_psgate = psgate;

	mixer_play(wav_teleport);
	psp = pac->psp;
	te_set_anim(psp, &am_cosmonaut_jump, 0, 0);
	te_set_anim_frame(psp, 1);
	pac->update = cosmonaut_ascend;
}

int cosmonaut_is_teleporting(void)
{
	struct actor *pac;

	pac = get_actor(AC_COSMONAUT);
	if (pac == NULL)
		return 0;

	if (pac->update == cosmonaut_ascend ||
	    pac->update == cosmonaut_disappear ||
	    pac->update == cosmonaut_teleport ||
	    pac->update == cosmonaut_appear)
	{
		return 1;
	}

	return 0;
}

void cosmonaut_do_walk_x(struct actor *pac, int *exit_side,
	struct actor **ppsgate)
{
	int oldx;

	oldx = pac->psp->x;
	cosmonaut_move_x(pac, exit_side);
	*ppsgate = check_stargate(pac, oldx);
	if ((!s_got_oxigen || *exit_side == 0) && cosmo_keydown(KEYA) &&
		*ppsgate == NULL)
	{
		mixer_play(wav_jump);
		set_jump(pac, (pac->dir == RDIR) ? 1 : -1);
	}
}

void cosmonaut_walk(struct actor *pac)
{
	int exit_side;
	struct actor *psgate;

	psgate = NULL;
	exit_side = 0;
	if (cosmo_keydown(KDOWN)) {
		set_down(pac);
	} else if (cosmo_keydown(KRIGHT)) {
		if (!cosmo_keydown(KLEFT) && pac->dir == LDIR) {
			set_actor_dir(pac, RDIR);
			pac->vx = WALK_V;
		}
		cosmonaut_do_walk_x(pac, &exit_side, &psgate);
	} else if (cosmo_keydown(KLEFT)) {
		if (!cosmo_keydown(KRIGHT) && pac->dir == RDIR) {
			set_actor_dir(pac, LDIR);
			pac->vx = -WALK_V;
		}
		cosmonaut_do_walk_x(pac, &exit_side, &psgate);
	} else {
		set_idle(pac);
	}

	if (!is_training() && psgate != NULL) {
		set_ascend(pac, psgate);
	}

	if ((!s_got_oxigen || exit_side == 0) &&
		(pac->update == cosmonaut_walk ||
		 pac->update == cosmonaut_idle))
       	{
		if (exit_side != 0) {
			show_balloon(&fr_oxigen_shine_0);
		}
		if (cosmonaut_how_far_y(pac, 1) == 1) {
			set_fall(pac);
		} else {
			check_lava(pac);
			check_dying(pac);
		}
	}

	if (!is_training() && s_got_oxigen && exit_side != 0 &&
		pac->update == cosmonaut_walk)
       	{
		if (exit_side < 0) {
			schedule_map_exit(0);
		} else {
			schedule_map_exit(1);
		}
	}
}

static void set_walk(struct actor *pac)
{
	struct sprite *psp;

	psp = pac->psp;
	if (pac->dir == RDIR)
		pac->vx = WALK_V;
	else
		pac->vx = -WALK_V;
	pac->ax = 0;

	te_set_anim(psp, &am_cosmonaut_walk, 3, 1);
	pac->update = cosmonaut_walk;
}

void set_idle(struct actor *pac)
{
	te_set_anim(pac->psp, &am_cosmonaut_idle, AM_SEC, 1);
	pac->update = cosmonaut_idle;
}

void cosmonaut_down(struct actor *pac)
{
	check_dying(pac);
	if (pac->update != cosmonaut_down) {
		return;
	}

	if (cosmo_keydown(KLEFT)) {
		set_actor_dir(pac, LDIR);
	} else if (cosmo_keydown(KRIGHT)) {
		set_actor_dir(pac, RDIR);
	}

	if (!cosmo_keydown(KDOWN)) {
		set_idle(pac);
	}
}

static void set_down(struct actor *pac)
{
	te_set_anim(pac->psp, &am_cosmonaut_down, 1, 1);
	pac->update = cosmonaut_down;
}

void cosmonaut_idle(struct actor *pac)
{
	check_dying(pac);
	if (pac->update != cosmonaut_idle) {
		return;
	}

	if (cosmo_keydown(KLEFT)) {
		set_actor_dir(pac, LDIR);
	} else if (cosmo_keydown(KRIGHT)) {
		set_actor_dir(pac, RDIR);
	}

	if (cosmo_keydown(KDOWN)) {
		set_down(pac);
		return;
	} else if (cosmo_keydown(KEYA)) {
		mixer_play(wav_jump);
		if (cosmo_keydown(KLEFT))
			set_jump(pac, -1);
		else if (cosmo_keydown(KRIGHT))
			set_jump(pac, 1);
		else
			set_jump(pac, 0);
		return;
	}

	if (cosmo_keydown(KLEFT) || cosmo_keydown(KRIGHT))
		set_walk(pac);
}

int cosmonaut_is_dead(void)
{
	return s_cosmonaut_should_die;
}

static void cosmonaut_set_dying_ex(int die, int snd)
{
	if (snd && !s_cosmonaut_should_die && die) {
		mixer_play(wav_hit);
	}
	s_cosmonaut_should_die = die;
}

/* Set dying but never play sound. */
void cosmonaut_set_dying_ns(int die)
{
	cosmonaut_set_dying_ex(die, 0);
}

void cosmonaut_set_dying(int die)
{
	cosmonaut_set_dying_ex(die, 1);
}

struct actor *spawn_cosmonaut(int tx, int ty)
{
	struct sprite *psp;
	struct actor *pac;

	psp = te_get_sprite(SP_COSMONAUT);
	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW;
	te_set_anim(psp, &am_cosmonaut_idle, AM_SEC, 1);

	pac = get_actor(AC_COSMONAUT);
	pac->psp = psp;
	set_actor_dir(pac, RDIR);
	pac->update = cosmonaut_idle;

	cosmonaut_set_dying(0);

	return pac;
}

static struct actor *spawn_cosmonaut_fp(int tx, int ty)
{
	s_start_tx[0] = tx;
	s_start_ty[0] = ty;
	tokscanf(NULL, "iiiiiii",
		&s_start_tx[1], &s_start_ty[1],
		&s_start_tx[2], &s_start_ty[2],
	       	&s_start_tx[3], &s_start_ty[3],
		&s_default_start);

	if (s_default_start < 0 || s_default_start > 3)
		s_default_start = 0;

	return spawn_cosmonaut(tx, ty);
}

struct actor *spawn_dead_cosmonaut(int tx, int ty)
{
	struct sprite *psp;

	psp = te_get_sprite(SP_COSMONAUT);
	psp->x = tx * TE_VBTW;
	psp->y = ty * TE_VBTW;
	psp->pframe = &fr_cosmonaut_die_4;

	return NULL;
}

static struct actor *spawn_dead_cosmonaut_fp(int tx, int ty)
{
	return spawn_dead_cosmonaut(tx, ty);
}

/*
 * start_point:
 * 	0 to 3, restars the map and puts player in one of the four positions.
 * 	else restarts map from default position.
 */ 
void cosmonaut_start_at(int start_point)
{
	struct actor *pac;

	pac = get_actor(AC_COSMONAUT);
	if (start_point < 0 || start_point > 3)
		start_point = s_default_start;

	pac->psp->x = s_start_tx[start_point] * TE_VBTW;
	pac->psp->y = s_start_ty[start_point] * TE_VBTW;
	if (start_point & 1)
		set_actor_dir(pac, LDIR);
	else
		set_actor_dir(pac, RDIR);
}

void cosmonaut_start_at_gate(int gateid)
{
	int i;
	struct actor *pac, *psgate;

	pac = get_actor(AC_COSMONAUT);
	if (is_free_actor(pac))
		return;

	/* Find the gate to go in psgate, or NULL */
	for (i = AC_STARGATE_0; i < AC_STARGATE_LAST + 1; i++) {
		psgate = get_actor(i);
		if (psgate->update != NULL &&
			stargate_get_id(psgate) == gateid)
		{
			break;
		} else {
			psgate = NULL;
		}
	}

	if (psgate == NULL) {
		cosmonaut_start_at(0);
	} else {
		activate_stargate(psgate);
		set_appear(pac, psgate);
		s_arrow_dirpos = stargate_get_arrow_dirpos(psgate);
	}
}

/* Takes parrow actor and sets its sprite position and actor
 * direction to the position towards which the cosmonaut shoud exit.
 */
void cosmonaut_set_actor_exit_dirpos(struct actor *parrow)
{
	int dir;

	if (kassert_fails(parrow != NULL && parrow->psp != NULL))
		return;

	parrow->psp->x = s_start_tx[s_arrow_dirpos] * TE_VBTW;
	parrow->psp->y = s_start_ty[s_arrow_dirpos] * TE_VBTW;
	dir = (s_arrow_dirpos & 1) ? RDIR : LDIR;
	if (dir == LDIR)
		parrow->psp->x += TE_VBTW;
	set_actor_dir(parrow, dir);
}

/* state: 0 idle, 1 jumping, 2, walking */
void cosmonaut_set_for_demo(int x, int y, int dir, int state,
			    int ax, int vx, int ay, int vy)
{
	struct actor *pac;

	pac = get_actor(AC_COSMONAUT);
	if (pac == NULL)
		return;

	pac->psp->x = x;
	pac->psp->y = y;
	set_actor_dir(pac, dir);
	pac->ax = ax;
	pac->vx = vx;
	pac->ay = ay;
	pac->vy = vy;
	if (state == 1) {
		te_set_anim(pac->psp, &am_cosmonaut_jump, 0, 0);
		pac->update = cosmonaut_jump;
	} else if (state == 2) {
		te_set_anim(pac->psp, &am_cosmonaut_walk, 3, 1);
		pac->update = cosmonaut_walk;
	}
}

/*
 * 'pac' and 'psp' are backups.
 * exit_side is 0 left, 1 right, 2 up, 3 down.
 */
void cosmonaut_enter_map(struct actor *pac, struct sprite *psp, int exit_side)
{
	struct rect box;

	if (kassert_fails(pac != NULL && psp != NULL &&
			  psp->pframe != NULL &&
			  psp->pframe->pbbox != NULL))
	{
		return;
	}

	*get_actor(AC_COSMONAUT) = *pac;
	*pac->psp = *psp;
	pac = get_actor(AC_COSMONAUT);

	get_actor_bbox(&box, pac);
	if (exit_side == 0) {
		pac->psp->x = TE_VSCRW - (box.x - pac->psp->x + box.w);
	} else if (exit_side == 1) {
		pac->psp->x = -(box.x - pac->psp->x);
	}

	if (pac->update == cosmonaut_jump) {
		cosmonaut_jump_y(pac);
	}
}

static void on_map_init(int mapid)
{
	if (s_dielava_chan >= 0) {
		mixer_stop(s_dielava_chan);
		s_dielava_chan = -1;
	}
}

void cosmonau_init(void)
{
	register_bitmap(&bmp_cosmonau, "cosmonau", 1, 0x8080);
	register_bitmap(&bmp_diewater, "diewater", 1, 0);
	register_spawn_fn("cosmonaut", spawn_cosmonaut_fp);
	register_spawn_fn("deadcosmo", spawn_dead_cosmonaut_fp);
	register_map_init_fn(on_map_init);
	register_sound(&wav_jump, "jump");
	register_sound(&wav_teleport, "teleport");
	register_sound(&wav_diewater, "diewater");
	register_sound(&wav_hit, "hit");
	register_sound(&wav_dielava, "dielava");
	/* register_sound(&wav_floorhit, "drop"); */
}
