#include "balloon.h"
#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "bitmaps.h"
#include "cbase/kassert.h"
#include <string.h>

static struct bmp *bmp_balloon = NULL; 

static FRAME(balloon, bmp_balloon, 0, 0, 32, 32)

enum {
	BALLOON_TIME = FPS * 5
};

static void follow_cosmonaut(struct actor *pac)
{
	struct actor *pcosmo;
	struct sprite *psp;

	pcosmo = get_actor(AC_COSMONAUT);
	if (kassert_fails(pcosmo->psp != NULL && pac->psp != NULL &&
		pac->psp->pframe != NULL))
	{
		return;
	}

	pac->psp->x = pcosmo->psp->x;
	pac->psp->y = pcosmo->psp->y - te_p2w(fr_balloon.r.h);

	psp = te_get_sprite(SP_BALLOON_ICON);
	if (kassert_fails(psp->pframe != NULL))
		return;

	psp->x = pac->psp->x + (te_p2w(pac->psp->pframe->r.w) / 2)
	       	- (te_p2w(psp->pframe->r.w) / 2);
	psp->y = pac->psp->y + (te_p2w(pac->psp->pframe->r.h) / 2)
	       	- (te_p2w(psp->pframe->r.h) / 2) - te_p2w(2);
}

static void balloon_idle(struct actor *pac)
{
	follow_cosmonaut(pac);
	pac->t = dectime(pac->t);
	if (pac->t <= 0) {
		hide_balloon();
	}
}

void hide_balloon(void)
{
	te_free_sprite(te_get_sprite(SP_BALLOON));
	te_free_sprite(te_get_sprite(SP_BALLOON_ICON));
	free_actor(get_actor(AC_BALLOON));
}

void show_balloon(struct frame *icon)
{
	struct sprite *psp;
	struct actor *pac;

	if (kassert_fails(icon != NULL))
		return;

	psp = te_get_sprite(SP_BALLOON_ICON);
	psp->pframe = icon;
	psp->flags = SP_F_TOP;

	pac = get_actor(AC_BALLOON);
	pac->t = BALLOON_TIME;
	psp = te_get_sprite(SP_BALLOON);
	psp->pframe = &fr_balloon;
	psp->flags = SP_F_TOP;
	pac->psp = psp;
	pac->update = balloon_idle;
	follow_cosmonaut(pac);
}

void balloon_init(void)
{
	register_bitmap(&bmp_balloon, "balloon", 1, 128);
}
