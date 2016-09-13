/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "msgbox.h"
#include "tilengin.h"
#include "strdraw.h"
#include "input.h"
#include "text.h"
#include "sounds.h"
#include "gamelib/mixer.h"
#include "cbase/kassert.h"
#include <string.h>

struct wav *wav_opsel;
struct wav *wav_opmove;

const static struct msgbox *s_mbox;

static int s_options_y;
static int s_options_x;
static int s_option;
static int s_noptions;

static void precalc(void)
{
	int w, y, maxw, i;

	y = s_mbox->y;
	if (s_mbox->title != NULL)
		y += 2;

	s_options_y = y;

	maxw = 0;
	s_noptions = 0;
	for (i = 0; i < NELEMS(s_mbox->options); i++) {
		if (s_mbox->options[i].str == NULL)
			break;
		s_noptions++;
		w = utf8_strlen(_(s_mbox->options[i].str));
		if (w > maxw)
			maxw = w;
	}

	s_options_x = (s_mbox->w - maxw) / 2;
	s_options_x += s_mbox->x;
}

static void draw_box(void)
{
	int x, y, h;
	int sx, sy, sw;
	int id;

	sx = s_mbox->x - 1;
	sw = s_mbox->w + 2;
	sy = s_mbox->y - 1;
	h = 1 + s_options_y - s_mbox->y + s_noptions * 2;

	/* corners */
	te_set_fg_xy(sx, sy, 1, chri('a'));
	te_set_fg_xy(sx + sw - 1, sy, 1, chri('b'));
	te_set_fg_xy(sx, sy + h - 1, 1, chri('c'));
	te_set_fg_xy(sx + sw - 1, sy + h - 1, 1, chri('d'));

	/* borders */
	id = chri('e');
	for (x = 1; x < sw - 1; x++)
		te_set_fg_xy(sx + x, sy, 1, id);

	id = chri('g');
	for (x = 1; x < sw - 1; x++)
		te_set_fg_xy(sx + x, sy + h - 1, 1, id);

	id = chri('h');
	for (y = 1; y < h - 1; y++)
		te_set_fg_xy(sx, sy + y, 1, id);

	id = chri('f');
	for (y = 1; y < h - 1; y++)
		te_set_fg_xy(sx + sw - 1, sy + y, 1, id);

	/* center */
	id = chri(' ');
	for (y = 1; y < h - 1; y++) {
		for (x = 1; x < sw - 1; x++) {
			te_set_fg_xy(sx + x, sy + y, 1, id);
		}
	}
}

static void draw_msgbox(void)
{
	int x, w, i, y;

	draw_box();

	y = s_mbox->y;
	if (s_mbox->title != NULL) {
		w = utf8_strlen(_(s_mbox->title));
		x = (s_mbox->w - w) / 2;
		draw_str(_(s_mbox->title), s_mbox->x + x, y, 1); 
	}

	y = s_options_y;
	for (i = 0; i < s_noptions; i++) {
		if (!kassert_fails(s_mbox->options[i].str != NULL)) {
			draw_str(_(s_mbox->options[i].str), s_options_x, y, 1);
		}
		y += 2;
	}
}

static void clear_msgbox(void)
{
	int x, y, h;
	int id_space;
	int sx, sy, sw;

	id_space = chri(' ');
	sx = s_mbox->x - 1;
	sy = s_mbox->y - 1;
	sw = s_mbox->w + 2;
	h = 1 + s_options_y - s_mbox->y + s_noptions * 2;
	for (y = 0; y < h; y++) {
		for (x = 0; x < sw; x++) {
			te_set_fg_xy(sx + x, sy + y, 0, id_space);
		}
	}
}

static void draw_arrow(void)
{
	te_set_fg_xy(s_options_x - 2, s_options_y + s_option * 2, 1,
		     chri(CHR_ARROW_R));
}

static void clear_arrow(void)
{
	te_set_fg_xy(s_options_x - 2, s_options_y + s_option * 2, 1,
		     chri(' '));
}

static void next_option(void)
{
	mixer_play(wav_opmove);
	clear_arrow();
	s_option = (s_option + 1) % s_noptions;
	draw_arrow();
}

static void prev_option(void)
{
	mixer_play(wav_opmove);
	clear_arrow();
	if (s_option == 0)
		s_option = s_noptions - 1;
	else
		s_option--;
	draw_arrow();
}

int update_msgbox(void)
{
	if (s_mbox == NULL)
		return MSGBOX_NONE;

	if (s_mbox->can_go_back &&
		(is_first_pressed(LKEYY) || is_first_pressed(LKEYB)))
	{
		clear_msgbox();
		return MSGBOX_BACK;
	} else if (is_first_pressed(LKEYA)) {
		clear_msgbox();
		return s_mbox->options[s_option].code;
	} else if (is_first_pressed(LKDOWN)) {
		next_option();
	} else if (is_first_pressed(LKUP)) {
		prev_option();
	}

	return MSGBOX_IDLE;
}

void show_msgbox(const struct msgbox *mbox)
{
	s_mbox = mbox;
	if (s_mbox == NULL)
		return;
	s_option = 0;
	precalc();
	draw_msgbox();
	draw_arrow();
}

void msgbox_init(void)
{
	register_sound(&wav_opsel, "opsel");
	register_sound(&wav_opmove, "opmove");
}
