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

#include "demosave.h"
#include "tilengin.h"
#include "game.h"
#include "input.h"
#include "cheats.h"
#include "cosmonau.h"
#include "gplay_st.h"
#include "gamelib/state.h"
#include "kernel/kernel.h"

#include <stdio.h>

static int s_frames;
static FILE *s_fp;
static int s_keys;
static int s_nkeys;
static int s_mapid;

static void update_demosave(struct actor *pac)
{
	int i, oldframes;
	struct actor *pcosmo;

	oldframes = s_frames++;
	if (oldframes == 0) {
		if ((s_fp = fopen("demosave.txt", "w")) == NULL)
			return;

		fprintf(s_fp, "map %d\n", s_mapid);

		/* save cosmonaut */
		pcosmo = get_actor(AC_COSMONAUT);
		if (pcosmo != NULL) {
			fprintf(s_fp, "x %d y %d dir %d\n", pcosmo->psp->x,
				pcosmo->psp->y, pcosmo->dir);
			fprintf(s_fp, "jump %d walk %d\n",
				pcosmo->update == cosmonaut_jump,
				pcosmo->update == cosmonaut_walk);
			fprintf(s_fp, "ax %d vx %d ay %d vy %d\n",
				pcosmo->ax, pcosmo->vx,
				pcosmo->ay, pcosmo->vy);
		}
	}

	if (s_fp == NULL)
		return;

	if (kernel_get_device()->key_first_pressed(KERNEL_KSC_S)) {
		fprintf(s_fp, "~\n");
		fclose(s_fp);
		s_fp = NULL;
	}

	if (s_fp == NULL)
		return;

	s_nkeys = 0;
	for (i = KUP; i < KEYB; i++)
		s_nkeys |= is_key_down(i) << i;

	if (oldframes == 0 || s_nkeys != s_keys) {
		fprintf(s_fp, "%d %d\n", oldframes, s_nkeys);
		s_keys = s_nkeys;
	}
}

static void spawn_demosave(int mapid)
{
	struct actor *pac;

	if (s_fp != NULL) {
		fprintf(s_fp, "~\n");
		fclose(s_fp);
		s_fp = NULL;
	}

	if (g_state != &gplay_st)
		return;

	pac = get_actor(AC_DEMOSAVE);
	if (!is_free_actor(pac))
		return;

	s_mapid = mapid;
	s_frames = 0;
	pac->update = update_demosave;
}

void demosave_init(void)
{
	if (s_cheats_debug_mode && s_cheats_demosave_mode) {
		register_map_init_fn(spawn_demosave);
	}
}
