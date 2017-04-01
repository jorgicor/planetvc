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

#include "gameover.h"
#include "tilengin.h"
#include "game.h"
#include "strdraw.h"
#include "text.h"

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

enum {
	STATE_WORD_COLOR_DELAY,
	STATE_CHAR_COLOR_DELAY
};

enum {
	DELAY_WORD_COLOR = FPS_FULL * 7,
	DELAY_CHAR_COLOR = 8,
};

static int s_state;
static int s_color_t;
static int s_color_char;
static int s_colors[] = { 0 };
static int s_color;
static int s_ypos;

static void gameover_update(struct actor *pac)
{
	char s[2];
	const char *str;
	int x, len;

	s_color_t = dectime(s_color_t);
	if (s_color_t > 0)
		return;

	if (s_state == STATE_WORD_COLOR_DELAY) {
		te_fill_fg(0, s_ypos, TE_FMW, 1, 0, chri(' '));
		s_color = (s_color + 1) % NELEMS(s_colors);
		s_color_char = 0;
		s_color_t = DELAY_CHAR_COLOR;
		s_state = STATE_CHAR_COLOR_DELAY;
	} else {
		str = _("GAME_OVER"); 
		len = utf8_strlen(str);
		if (s_color_char >= len) {
			s_color_t = DELAY_WORD_COLOR;
			s_state = STATE_WORD_COLOR_DELAY;
			return;
		}
		x = (TE_FMW - len) / 2;
		s[0] = str[s_color_char]; 
		s[1] = '\0';
		draw_str(s, x + s_color_char, s_ypos, s_colors[s_color]);
		s_color_char++;
		s_color_t = DELAY_CHAR_COLOR;
	}
}

struct actor *spawn_gameover(int x, int y)
{
	struct actor *pac;

	pac = get_actor(AC_GAMEOVER);
	if (!is_free_actor(pac))
		return NULL;

	s_color = 0;
	s_color_char = 0;
	s_color_t = DELAY_CHAR_COLOR;
	s_state = STATE_CHAR_COLOR_DELAY;
	s_ypos = y;
	te_fill_fg(0, s_ypos, TE_FMW, 1, 0, chri(' '));
	pac->update = gameover_update;
	return pac;
}

struct actor *spawn_gameover_fp(int x, int y)
{
	return spawn_gameover(x, y);
}

void gameover_init(void)
{
	register_spawn_fn("gameover", spawn_gameover_fp);
}
