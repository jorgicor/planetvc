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

static int s_msgbox_y;
static int s_options_y;
static int s_options_x;
static int s_option;
static int s_noptions;
static int s_title_h;

static struct {
	short tseti;
	short tilei;
} s_backup[TE_FMW][TE_FMH];

static void precalc(void)
{
	int w, h, maxw, i;

	h = 0;
	if (s_mbox->icon != NULL) {
		h += 3;
	}

	if (s_mbox->title != NULL) {
		s_title_h = str_height_ww(_(s_mbox->title), s_mbox->x,
			s_mbox->w);
		h += s_title_h * 2;
	}

	s_options_y = h;

	maxw = 0;
	s_noptions = 0;
	for (i = 0; i < NELEMS(s_mbox->options); i++) {
		if (s_mbox->options[i].str == NULL) {
			break;
		}
		s_noptions++;
		h += 2;
		w = utf8_strlen(_(s_mbox->options[i].str));
		if (w > maxw) {
			maxw = w;
		}
	}

	s_options_x = (s_mbox->w - maxw) / 2;
	s_options_x += s_mbox->x;
	s_msgbox_y = (TE_FMH - h) / 2;
	s_options_y += s_msgbox_y;
}

static void save_background(void)
{
	int x, y;
	int tseti, tilei;

	for (x = 0; x < TE_FMW; x++) {
		for (y = 0; y < TE_FMH; y++) {
			te_fg_xy(x, y, &tseti, &tilei);
			s_backup[x][y].tseti = tseti;
			s_backup[x][y].tilei = tilei;
		}
	}
}

static void restore_background(void)
{
	int x, y;

	for (x = 0; x < TE_FMW; x++) {
		for (y = 0; y < TE_FMH; y++) {
			te_set_fg_xy(x, y,
				     s_backup[x][y].tseti,
				     s_backup[x][y].tilei);
		}
	}
}

static void draw_box(void)
{
	int x, y, h;
	int sx, sy, sw;
	int id;

	sx = s_mbox->x - 1;
	sw = s_mbox->w + 2;
	sy = s_msgbox_y - 1;
	h = 1 + s_options_y - s_msgbox_y + s_noptions * 2;

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

	y = s_msgbox_y;
	if (s_mbox->icon != NULL) {
		x = (s_mbox->w - 2) / 2;
		draw_icon(s_mbox->icon, s_mbox->x + x, y, 1);
		y += 3;
	}

	if (s_mbox->title != NULL) {
		if (s_title_h == 1) {
			w = utf8_strlen(_(s_mbox->title));
			x = (s_mbox->w - w) / 2;
			draw_str(_(s_mbox->title), s_mbox->x + x, y, 1); 
		} else {
			draw_str_ww(_(s_mbox->title), s_mbox->x, y, 1, -1,
				    s_mbox->w);
		}
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
	sy = s_msgbox_y - 1;
	sw = s_mbox->w + 2;
	h = 1 + s_options_y - s_msgbox_y + s_noptions * 2;
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
	int code;

	if (s_mbox == NULL)
		return MSGBOX_NONE;

	if (s_mbox->can_go_back &&
		(is_first_pressed(LKEYY) || is_first_pressed(LKEYB)))
	{
		clear_msgbox();
		restore_background();
		s_mbox = NULL;
		return MSGBOX_BACK;
	} else if (is_first_pressed(LKEYA)) {
		clear_msgbox();
		restore_background();
		code = s_mbox->options[s_option].code;
		s_mbox = NULL;
		return code;
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
	if (s_mbox == NULL) {
		return;
	}
	s_option = 0;
	precalc();
	save_background();
	draw_msgbox();
	draw_arrow();
}

void msgbox_init(void)
{
	register_sound(&wav_opsel, "opsel");
	register_sound(&wav_opmove, "opmove");
}
