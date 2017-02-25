/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "stargate.h"
#include "tilengin.h"
#include "game.h"
#include "data.h"
#include "bitmaps.h"
#include "cbase/cbase.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"

#ifdef HAVE_TGMATH_H
#	include <tgmath.h>
#else
#	include <math.h>
#endif

#define ID			i0
#define GOTO_MAPID		i1
#define ARROW_DIRPOS		i2
#define TX			vx
#define TY			vy
#define GOTO_GATEID		ax
#define NEXT_ACTIVE_GATEID	ay

enum {
	PLASMAW = 64,
	PALSZ = 13,
	PALSZ_DIV_2 = PALSZ / 2,
	MASK_OFFS = 16,
	STEPS = 8
};

static struct bmp *bmp_stargate = NULL;

static FRAME(stargate, bmp_stargate, 0, 0, 96, 96)
static FRAME(plasma_mask, bmp_stargate, 112, 16, PLASMAW, PLASMAW)

/* ID of the main active stargate. */
int s_active_stargate_id;

/* If the plasma bitmap has been initialized. Done only once. */
static int s_init_plasma = 1;

static unsigned char s_plasma_pixels[PLASMAW * PLASMAW];

static unsigned int s_plasma_pal[PALSZ + 1] = {
	0x000080,
	0x0000ff,
	0x8000ff,
	0x0080ff,
	0x00ffff,
	0x80ffff,
	0xffffff,
	0x80ffff,
	0x00ffff,
	0x0080ff,
	0x8000ff,
	0x0000ff,
	0x000080,
	0x000000
};

static struct bmp s_bmp_plasma = {
	.pixels = s_plasma_pixels,
	.w = PLASMAW,
	.h = PLASMAW,
	.pitch = PLASMAW,
	.pal = s_plasma_pal,
	.palsz = PALSZ + 1,
	.use_key_color = 1,
	.key_color = 0
};

static struct bmp *bmp_plasma = &s_bmp_plasma;

static FRAME(plasma, bmp_plasma, 0, 0, PLASMAW, PLASMAW)

static int calc_color(int x, int y)
{
	float c;

	c = PALSZ_DIV_2 + (PALSZ_DIV_2 * sin(x / 4.f));
	c += PALSZ_DIV_2 + (PALSZ_DIV_2 * sin(y / 2.f));
	c += PALSZ_DIV_2 + (PALSZ_DIV_2 * sin((x + y) / 4.f));
	c += PALSZ_DIV_2 + (PALSZ_DIV_2 * sin(sqrt((float) x*x + y*y)) / 2.f);
	return (int) (c / 4);
}

static void plasma_init(void)
{
	int x, y, colori;
	int si, dx, i, srcbytes; 

	i = 0;
	for (y = 0; y < PLASMAW; y++) {
		for (x = 0; x < PLASMAW; x++) {
			colori = calc_color(x, y);
			if (kassert_fails(colori >= 0 && colori < PALSZ))
				colori = 0;
			s_plasma_pixels[i++] = colori;
		}
	}

	/* copy mask */
	si = fr_plasma_mask.r.y * bmp_stargate->pitch + fr_plasma_mask.r.x;
	dx = bmp_stargate->pitch - PLASMAW;
	srcbytes = bmp_stargate->pitch * bmp_stargate->h;
	i = 0;
	for (y = 0; y < PLASMAW; y++) {
		kasserta(i < NELEMS(s_plasma_pixels) &&
			 si < srcbytes && si + PLASMAW <= srcbytes);
		for (x = 0; x < PLASMAW; x++) {
			if (bmp_stargate->pixels[si] == 0) {
				s_plasma_pixels[i] = PALSZ;
			}
			si++;
			i++;
		}
		si += dx;
	}
}

static void update_plasma(void)
{
	static int count = 0;
	int i, tmp;

	if (s_init_plasma) {
		plasma_init();
		s_init_plasma = 0;
	}

	count += FPS_MUL;
	if (count < STEPS)
		return;
	else
		count = 0;

	/* cycle palette */
	tmp = s_plasma_pal[PALSZ - 1];
	for (i = PALSZ - 1; i > 0; i--) {
		s_plasma_pal[i] = s_plasma_pal[i - 1];
	}
	s_plasma_pal[0] = tmp;
}

static void stargate_idle(struct actor *pac)
{
}

static void stargate_active(struct actor *pac)
{
}

void deactivate_stargate(struct actor *pac)
{
	if (pac->psp != NULL) {
		te_free_sprite(pac->psp);
		pac->psp = NULL;
	}
	pac->update = stargate_idle;
}

void activate_stargate(struct actor *pac)
{
	struct sprite *psp;

	if (pac->psp != NULL) {
		te_free_sprite(pac->psp);
		pac->psp = NULL;
	}

	psp = te_get_free_sprite(SP_STARGATE_0, SP_STARGATE_LAST + 1);
	if (psp == NULL)
		return;

	psp->x = (pac->TX + 1) * TE_VBTW;
	psp->y = (pac->TY + 1) * TE_VBTW; 
	psp->pframe = &fr_plasma;
	pac->psp = psp;
	pac->update = stargate_active;
}

static struct actor *spawn_stargate(int tx, int ty, int id, int goto_mapid,
				    int goto_gateid, int next_active_gateid,
				    int active, int arrow_dirpos)
{
	struct actor *pac;

	pac = get_free_actor(AC_STARGATE_0, AC_STARGATE_LAST + 1);
	if (pac == NULL)
		return NULL;

	if (kassert_fails(arrow_dirpos >= 0 && arrow_dirpos <= 3))
		arrow_dirpos = 1;

	pac->ID = id;
	pac->GOTO_MAPID = goto_mapid;
	pac->GOTO_GATEID = goto_gateid;
	pac->NEXT_ACTIVE_GATEID = next_active_gateid;
	pac->TX = tx;
	pac->TY = ty;
	pac->ARROW_DIRPOS = arrow_dirpos;
	if (id == s_active_stargate_id || active) {
		activate_stargate(pac);
	} else {
		deactivate_stargate(pac);
	}
	return pac;
}

static struct actor *spawn_stargate_fp(int tx, int ty)
{
	int id, goto_mapid, goto_gateid, next_active_gateid;
	int r, active, arrow_dirpos;

	r = tokscanf(NULL, "i", &id);
	if (r != 1) {
		id = -1;
	}

	r = tokscanf(NULL, "iii", &goto_mapid, &goto_gateid,
		&next_active_gateid);
	if (r != 3) {
		goto_mapid = -1;
		goto_gateid = -1;
		next_active_gateid = -1;
	}

	r = tokscanf(NULL, "i", &active);
	if (r != 1) {
		active = 0;
	}

	r = tokscanf(NULL, "i", &arrow_dirpos);
	if (r != 1) {
		arrow_dirpos = 1;
	}

	return spawn_stargate(tx, ty, id, goto_mapid, goto_gateid,
			      next_active_gateid, active, arrow_dirpos);
}

/* The map to go from this stargate. */
int stargate_get_goto_mapid(struct actor *pac)
{
	return pac->GOTO_MAPID;
}

/* The gate to go from this stargate. */
int stargate_get_goto_gateid(struct actor *pac)
{
	return pac->GOTO_GATEID;
}

/* The gate activate once we teleport through this gate. */
int stargate_get_next_active_gateid(struct actor *pac)
{
	return pac->NEXT_ACTIVE_GATEID;
}

/* Get our center position. */
void stargate_get_center(struct actor *pac, int *cx, int *cy)
{
	int x, y;

	x = (pac->TX * TE_VBTW) + te_p2w(fr_stargate.r.w / 2);
	y = (pac->TY * TE_VBTW) + te_p2w(fr_stargate.r.h / 2);
	if (cx != NULL)
		*cx = x;
	if (cy != NULL)
		*cy = y;
}

/* Gets 0, 1, 2, 3 for the screen exit position that the arrow indicating
 * position must show when the player takes oxigen. */
int stargate_get_arrow_dirpos(struct actor *pac)
{
	return pac->ARROW_DIRPOS;
}

int stargate_get_id(struct actor *pac)
{
	return pac->ID;
}

/* 
 * Checks if pcosmo passes horizontally from one side of the other
 * of our center position and we are the main active stargate.
 * Returns 1 if should teleport.
 */
int stargate_check_teleport(struct actor *pac, struct actor *pcosmo, int oldx)
{
	int newx, w2, midx;

	if (pac->update == stargate_idle)
		return 0;
	if (pac->ID != s_active_stargate_id)
		return 0;

	midx = pac->psp->x + te_p2w(pac->psp->pframe->r.w / 2);
	w2 = te_p2w(pcosmo->psp->pframe->r.w/2);
	oldx += w2;
	newx = pcosmo->psp->x + w2;
	if (isign(newx - midx) == isign(oldx - midx)) {
		/* He didn't pass from side to side */
		return 0;
	}

	if (pcosmo->psp->y > pac->psp->y &&
		pcosmo->psp->y < pac->psp->y +
			te_p2w(pac->psp->pframe->r.h))
	{
		return 1;
	}

	return 0;
}

void stargate_init(void)
{
	s_init_plasma = 1;
	register_bitmap(&bmp_stargate, "stargate", 1, 0x8000);
	register_spawn_fn("stargate", spawn_stargate_fp);
	register_update_fn(update_plasma);
}
