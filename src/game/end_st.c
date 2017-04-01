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

#include "end_st.h"
#include "tilengin.h"
#include "fade.h"
#include "fade_st.h"
#include "win_st.h"
#include "game.h"
#include "strdraw.h"
#include "initfile.h"
#include "google.h"
#include "cfg/cfg.h"

static void draw(void)
{
	te_begin_draw();
	te_draw();
	fade_draw();
	te_end_draw();
}

static void update(void)
{
	exec_update_fns();
	update_actors();
	exec_init_map_code();
}

static void enter(const struct state *old_state)
{
	static const char *last_achiev[NDIFFICULTY_LEVELS] = {
		"CgkIk_7jut4bEAIQEw",
		"CgkIk_7jut4bEAIQDQ"
	};

	if (!PP_DEMO && Android_IsConnectedToGooglePlay()) {
		Android_UnlockAchievement(last_achiev[get_difficulty()]);
	}

	load_map(initfile_getvar("end_map"));
}

static void end(void)
{
	fade_to_state(&win_st);
}

const struct state end_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.end = end
};
