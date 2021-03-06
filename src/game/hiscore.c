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

#include "hiscore.h"
#include "tilengin.h"
#include "game.h"
#include "strdraw.h"
#include "text.h"
#include "initfile.h"
#include "hud.h"
#include "input.h"
#include "win_st.h"
#include "msgbox.h"
#include "confpath.h"
#include "google.h"
#include "crypt.h"
#include "prefs.h"
#include "gamelib/mixer.h"
#include "gamelib/vfs.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

struct hiscore {
	char name[4];
	int nvisited;
};

static const struct hiscore s_hiscores_const[] = {
#if 1
	{ "ADC", 58 },
	{ "SBC", 42 }, 
	{ "NOP", 33 }, 
	{ "JMP", 24 }, 
	{ "RST", 16 },
	{ "XOR", 2 }, 
#else
	{ "ADC", 60 },
	{ "SBC", 60 }, 
	{ "NOP", 60 }, 
	{ "JMP", 60 }, 
	{ "RST", 60 },
	{ "XOR", 60 }, 
#endif
};

static struct hiscore s_hiscores[NELEMS(s_hiscores_const)];
static struct hiscore s_hiscores_beginner[NELEMS(s_hiscores_const)];

enum {
	NSCORES = NELEMS(s_hiscores)
};

static struct hiscore *s_hiscores_by[] = {
	s_hiscores,
	s_hiscores_beginner,
};

static const char *s_difficulty_name[] = {
       	"EXPERT",
        "BEGINNER"
};

const char *s_board_id[] = {
	"CgkIk_7jut4bEAIQBA",
	"CgkIk_7jut4bEAIQAQ"
};

static struct hiscore *s_hiscores_tab = s_hiscores;

static const char s_letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
enum {
	NLETTERS = sizeof(s_letters) - 1,
	BLINK_TIME = FPS / 3,
};
	
static int s_enter_hspos;	/* position in s_hiscores */
static int s_enter_chr_pos;	/* which of the three characters. */
static int s_enter_chr_num;	/* index into s_letters */
static int s_hiscore_y;
static int s_hiscore_nmaps;
static int s_blink;

static void encrypt_scores(struct hiscore table[])
{
	int i;

	for (i = 0; i < NSCORES; i++) {
		table[i].nvisited = encrypt(table[i].nvisited);
	}
}

/* Returns NULL if failure, or a pointer to path, that will be filled.
 * path must have space for OPEN_FILE_MAX_PATH_LEN + 1 characters.
 */
static char *mkpath(char path[])
{
	static const char *fname = "hiscore.txt";
	const char *cpath;

	cpath = confpath_get();
	if (cpath == NULL)
		return NULL;

	if (kassert_fails(strlen(cpath) + strlen(fname) <=
			  OPEN_FILE_MAX_PATH_LEN))
	{
		return NULL;
	}

	strcpy(path, cpath);
	strcat(path, fname);
	return path;
}

static void save_scores(FILE *fp, int difficulty)
{
	int i;

	for (i = 0; i < NSCORES; i++) {
		fprintf(fp, "%s %d\n", s_hiscores_by[difficulty][i].name,
			decrypt(s_hiscores_by[difficulty][i].nvisited));
	}
}

static void hiscore_save(void)
{
	int i;
	FILE *fp;
	char path[OPEN_FILE_MAX_PATH_LEN + 1];

	if (mkpath(path) == NULL)
		return;

	if ((fp = fopen(path, "w")) == NULL)
		return;

	for (i = 0; i < NDIFFICULTY_LEVELS; i++) {
		save_scores(fp, i);
	}

	fclose(fp);
}

static int load_scores(FILE *fp, int difficulty)
{
	int i, r;
	struct hiscore bscores[NSCORES];

	for (i = 0; i < NSCORES; i++) {
		r = fscanf(fp, " %3s %d ", bscores[i].name,
			&bscores[i].nvisited);
		if (r != 2) {
			return -1;
		}
	}

	for (i = 0; i < NSCORES; i++) {
		s_hiscores_by[difficulty][i] = bscores[i];
	}

	encrypt_scores(s_hiscores_by[difficulty]);

	return 0;
}

void hiscore_load(void)
{
	int i;
	FILE *fp;
	char path[OPEN_FILE_MAX_PATH_LEN + 1];

	if (mkpath(path) == NULL)
		return;

	if ((fp = fopen(path, "r")) == NULL)
		return;

	for (i = 0; i < NDIFFICULTY_LEVELS; i++) {
		if (load_scores(fp, i) == -1) {
			goto end;
		}
	}

end:	fclose(fp);
}

/* Retuns -1 if not a hiscore.
 * Else returns the index in the table where the hiscore must lay.
 */
static int get_hiscore_pos(int nvisited)
{
	int i;
	int all_atmax;

	all_atmax = 1;
	for (i = 0; i < NSCORES; i++) {
		if (decrypt(s_hiscores_tab[i].nvisited) != s_hiscore_nmaps)
		{
			all_atmax = 0;
		}
		if (nvisited >= decrypt(s_hiscores_tab[i].nvisited))
			return i;
	}

	/* If all the scores are the maximum, allow to put a hiscore
	 * at least in the last entry... */
	if (all_atmax) {
		return NSCORES - 1;
	}

	return -1;
}

static void draw_hiscore(int i, int y, int total)
{
	char str[TE_FMW + 1];

	snprintf(str, sizeof(str), "%d. %s     %2d/%d", i + 1,
		 s_hiscores_tab[i].name,
		 decrypt(s_hiscores_tab[i].nvisited), total);
	draw_str(str, (TE_FMW - strlen(str)) / 2, y, 0);
}

static void draw_hiscores(int y)
{
	int i;

	for (i = 0; i < NSCORES; i++) {
		draw_hiscore(i, y, s_hiscore_nmaps);
		y += 2;
	}
}

static void clear_hint(int y)
{
	te_fill_fg(0, y, TE_FMW, 2, 0, chri(' '));
}

static void draw_mode(int y, int difficulty)
{
	int x;
	const char *str;

	str = _(s_difficulty_name[difficulty]);
	x = (TE_FMW - utf8_strlen(str)) / 2;
	draw_str(str, x, y, 1);
}

static void draw_hint(int y)
{
	int x;
	const char *str;

	if (PP_PHONE_MODE) {
		str = "USE i OR j";
	} else {
		str = "USE i j OR 'SPACE'";
	}
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, y, 1);

	if (PP_PHONE_MODE) {
		str = "'A' TO SELECT";
	} else {
		str = "'ENTER' TO SELECT";
	}
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, y + 1, 1);
}

static void hiscore_void(struct actor *pac)
{
}

static void redraw_char_pos(struct actor *pac)
{
	s_hiscores_tab[s_enter_hspos].name[s_enter_chr_pos] =
		s_letters[s_enter_chr_num];
	draw_hiscore(s_enter_hspos, s_hiscore_y + s_enter_hspos * 2,
		     s_hiscore_nmaps);
	pac->t = BLINK_TIME * 2;
	s_blink = 1;
}

static void clear_char_pos(void)
{
	s_hiscores_tab[s_enter_hspos].name[s_enter_chr_pos] = ' ';
	draw_hiscore(s_enter_hspos, s_hiscore_y + s_enter_hspos * 2,
		     s_hiscore_nmaps);
}

static void sendScore(int difficulty, int nvisited, int nlifes)
{
	int n;
	char scorestr[16];

	if (Android_IsConnectedToGooglePlay()) {
		n = nvisited * 100 + nlifes;
		snprintf(scorestr, sizeof(scorestr), "%d", n);
		Android_SendScore(s_board_id[difficulty], scorestr);
		Android_ShowLeaderboard(s_board_id[difficulty]);
	}
}

static void hiscore_wait_leaderboard(struct actor *pac)
{
	if (!Android_IsRequestingLeaderboard()) {
		pac->update = hiscore_void;
		if (!Android_IsConnectedToGooglePlay()) {
			set_preference_int("autoconnect", 0);
			save_prefs();
		}
	}
}

static void hiscore_enter(struct actor *pac)
{
	pac->t = dectime(pac->t);
	if (pac->t <= 0) {
		pac->t = BLINK_TIME;
		s_blink ^= 1;
		if (s_blink) {
			redraw_char_pos(pac);
		} else {
			clear_char_pos();
		}
	}

	if (is_first_pressed(LKUP)) {
		mixer_play(wav_opmove);
		s_enter_chr_num--;
		if (s_enter_chr_num < 0)
			s_enter_chr_num = NLETTERS - 1;
		redraw_char_pos(pac);
	} else if (is_first_pressed(LKDOWN) ||
		kernel_get_device()->key_first_pressed(KERNEL_KSC_SPACE))
       	{
		mixer_play(wav_opmove);
		s_enter_chr_num = (s_enter_chr_num + 1) % NLETTERS;
		redraw_char_pos(pac);
	}

	if (is_first_pressed(LKEYA)) {
		mixer_play(wav_opsel);
		redraw_char_pos(pac);
		s_enter_chr_pos++;
		s_enter_chr_num = 0;
		if (s_enter_chr_pos >= 3) {
			clear_hint(TE_FMH - 2);
			hiscore_save();
			kernel_get_device()->clear_first_pressed_keys();
			if (Android_IsConnectedToGooglePlay()) {
				sendScore(get_difficulty(),
					decrypt(s_hiscores_tab[s_enter_hspos]
						.nvisited),
					get_nlifes_remaining());
				pac->update = hiscore_wait_leaderboard;
			} else {
				pac->update = hiscore_void;
			}
		}
	}
}

static void set_hiscore_enter(struct actor *pac, int pos, int nvisited)
{
	int i;

	for (i = NSCORES - 1; i > pos; i--) {
		s_hiscores_tab[i] = s_hiscores_tab[i - 1];
	}
		
	strcpy(s_hiscores_tab[pos].name, "AAA");
	s_hiscores_tab[pos].nvisited = encrypt(nvisited);
	s_enter_hspos = pos;
	s_enter_chr_pos = 0;
	s_enter_chr_num = 0;
	draw_hiscores(s_hiscore_y);
	pac->t = BLINK_TIME;
	s_blink = 1;
	draw_hint(TE_FMH - 2);
	pac->update = hiscore_enter;
}

static void set_hiscore_void(struct actor *pac)
{
	draw_hiscores(s_hiscore_y);
	pac->update = hiscore_void;
}

int hiscore_is_working(void)
{
	struct actor *pac;

	pac = get_actor(AC_HISCORE);
	if (is_free_actor(pac))
		return 0;

	if (pac->update != hiscore_void)
		return 1;

	return 0;
}

struct actor *spawn_hiscore(int x, int y)
{
	struct actor *pac;
	int pos, n;

	pac = get_actor(AC_HISCORE);
	if (!is_free_actor(pac))
		return NULL;

	draw_mode(6, get_difficulty());
	s_hiscores_tab = s_hiscores_by[get_difficulty()];
	s_hiscore_y = y;
	s_hiscore_nmaps = initfile_getvar("nmaps_to_win");
	n = get_nvisited_maps();
	if (g_state == &win_st) {
		/* We passed the game, n should be one more. */
		n++;
	}
	if (n > s_hiscore_nmaps) {
		n = s_hiscore_nmaps;
	}
	pos = get_hiscore_pos(n);
	if (pos >= 0) {
		set_hiscore_enter(pac, pos, n);
	} else {
		set_hiscore_void(pac);
	}

	return pac;
}

struct actor *spawn_hiscore_fp(int x, int y)
{
	return spawn_hiscore(x, y);
}

void hiscore_init(void)
{
	kasserta(NSCORES == NELEMS(s_hiscores));
	kasserta(NSCORES == NELEMS(s_hiscores_beginner));
	kasserta(NDIFFICULTY_LEVELS == NELEMS(s_hiscores_by));
	kasserta(NDIFFICULTY_LEVELS == NELEMS(s_difficulty_name));

	/* Init mutable hiscores */
	memcpy(s_hiscores, s_hiscores_const, sizeof(s_hiscores));
	memcpy(s_hiscores_beginner, s_hiscores_const,
	       sizeof(s_hiscores_beginner));

	encrypt_scores(s_hiscores);
	encrypt_scores(s_hiscores_beginner);

	register_spawn_fn("hiscore", spawn_hiscore_fp);
}
