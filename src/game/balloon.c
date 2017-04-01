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
