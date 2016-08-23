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
