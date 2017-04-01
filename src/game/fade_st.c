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

#include "fade_st.h"
#include "game.h"
#include "input.h"
#include "tilengin.h"
#include "fade.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"

static const struct state *s_state_to_fade = NULL;

static void draw(void)
{
	te_begin_draw();
	te_draw();
	fade_draw();
	te_end_draw();
}

static void update(void)
{
	fade_update();
	if (is_fade_over()) {
		te_enable_anims(1);
		switch_to_state(s_state_to_fade);
	}
}

static void enter(const struct state *old_state)
{
	te_enable_anims(0);
	fade_out();
}

const struct state fade_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
};

void fade_to_state(const struct state *st)
{
	kasserta(st != NULL);
	s_state_to_fade = st;
	switch_to_state(&fade_st);
}
