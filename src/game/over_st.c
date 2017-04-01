/*
Copyright (c) 2014-2017 Jorge Giner Cordero

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

#include "over_st.h"
#include "game.h"
#include "tilengin.h"
#include "fade_st.h"
#include "menu_st.h"
#include "cheats.h"
#include "modplay.h"
#include "initfile.h"
#include "buy_st.h"
#include "kernel/kernel.h"

static void draw(void)
{
	te_begin_draw();
	te_draw();
	te_end_draw();
}

static void update(void)
{
	const struct kernel_device *d;

	d = kernel_get_device();
	if (s_cheats_debug_mode && d->key_first_pressed(KERNEL_KSC_R)) {
		load_map(initfile_getvar("over_map"));
	}

	exec_update_fns();
	update_actors();
	exec_init_map_code();
}

static void enter(const struct state *old_state)
{
	load_map(initfile_getvar("over_map"));
}

static void end(void)
{
	if (initfile_getvar("demo_version")) {
		fade_to_state(&buy_st);
	} else {
		fade_to_state(&menu_st);
	}
}

static void leave(const struct state *new_state)
{
	// modplay_stop();
}

const struct state over_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end,
	.leave = leave,
};
