/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "menu_st.h"
#include "game.h"
#include "input.h"
#include "text.h"
#include "tilengin.h"
#include "data.h"
#include "fade_st.h"
#include "gplay_st.h"
#include "demo_st.h"
#include "strdraw.h"
#include "oxigen.h"
#include "cheats.h"
#include "prefs.h"
#include "sounds.h"
#include "modplay.h"
#include "msgbox.h"
#include "menu.h"
#include "initfile.h"
#include "cheats.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif
#include <string.h>

static struct wav *wav_play;

#define TX_PLAY		"PLAY"
#define TX_OPTIONS	"OPTIONS"
#define TX_LANG		"LANGUAGE"
#define TX_HELP		"HELP"
#define TX_EXIT		"QUIT"

#define TX_REDEFINE	"DEFINE KEYS"
#define TX_MUSIC_ON	"MUSIC ON"
#define TX_MUSIC_OFF	"MUSIC OFF"
#define TX_DEMO		"DEMO"
#define TX_CREDITS	"CREDITS"
#define TX_BACK		"BACK"

enum {
	OP_PLAY,
	OP_OPTIONS,
	OP_LANG,
	OP_HELP,
	OP_EXIT,
	OP_REDEFINE,
	OP_MUSIC,
	OP_DEMO,
	OP_CREDITS,
	OP_BACK,
};

enum {
	MAIN_MENU_Y = 13,
	OPTIONS_MENU_Y = 13,
	OP_MUSIC_INDEX = 1,
};

static struct menu s_main_menu = {
	.options = {
		{ TX_PLAY, OP_PLAY },
		{ TX_OPTIONS, OP_OPTIONS },
		{ TX_LANG, OP_LANG },
		{ TX_HELP, OP_HELP },
		{ TX_EXIT, OP_EXIT },
		{ NULL, -2 },
	},
};

static struct menu s_options_menu_nodemo = {
	.options = {
		{ TX_REDEFINE, OP_REDEFINE },
		{ TX_MUSIC_OFF, OP_MUSIC },
		{ TX_CREDITS, OP_CREDITS },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static struct menu s_options_menu = {
	.options = {
		{ TX_REDEFINE, OP_REDEFINE },
		{ TX_MUSIC_OFF, OP_MUSIC },
		{ TX_DEMO, OP_DEMO },
		{ TX_CREDITS, OP_CREDITS },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static const char *s_options_menu_add[] = {
	TX_MUSIC_ON,
	TX_MUSIC_OFF,
	NULL
};

enum {
	STATE_MAIN_MENU,
	STATE_REDEFINE,
	STATE_LANG,
};

static int s_state;

enum {
	PRESS_KEY_Y = TE_FMH - 4,
};

static int s_redefine_keyi;

struct redefine_info {
	const char *str;
	const char *pref_name;
	int game_key;
};

static struct redefine_info s_redefine_info[] = {
	{ "PRESS KEY FOR JUMP", "keya", KEYA },
	{ "PRESS KEY FOR CROUCH", "kdown", KDOWN },
	{ "PRESS KEY FOR LEFT", "kleft", KLEFT },
	{ "PRESS KEY FOR RIGHT", "kright", KRIGHT },
	{ "PRESS KEY FOR 'RESTART'", "keyb", KEYB },
	{ "PRESS KEY FOR 'PRACTICE'", "keyx", KEYX },
};

/* Here we store the scan codes of the keys
 * that the user goes redefining, so he can't
 * select one selected before.
 */
static int s_redefined_keys[NELEMS(s_redefine_info)];

/* Allowed keys for redefinition. */
static const int s_allowed_keys[] = {
	KERNEL_KSC_0,
	KERNEL_KSC_1,
	KERNEL_KSC_2,
	KERNEL_KSC_3,
	KERNEL_KSC_4,
	KERNEL_KSC_5,
	KERNEL_KSC_6,
	KERNEL_KSC_7,
	KERNEL_KSC_8,
	KERNEL_KSC_9,
	KERNEL_KSC_A,
	KERNEL_KSC_B,
	KERNEL_KSC_C,
	KERNEL_KSC_D,
	KERNEL_KSC_E,
	KERNEL_KSC_F,
	KERNEL_KSC_G,
	KERNEL_KSC_H,
	KERNEL_KSC_I,
	KERNEL_KSC_J,
	KERNEL_KSC_K,
	KERNEL_KSC_L,
	KERNEL_KSC_M,
	KERNEL_KSC_N,
	KERNEL_KSC_O,
	KERNEL_KSC_P,
	KERNEL_KSC_Q,
	KERNEL_KSC_R,
	KERNEL_KSC_S,
	KERNEL_KSC_T,
	KERNEL_KSC_U,
	KERNEL_KSC_V,
	KERNEL_KSC_W,
	KERNEL_KSC_X,
	KERNEL_KSC_Y,
	KERNEL_KSC_Z,
	KERNEL_KSC_LEFT,
	KERNEL_KSC_UP,
	KERNEL_KSC_RIGHT,
	KERNEL_KSC_DOWN,
	KERNEL_KSC_LSHIFT,
	KERNEL_KSC_RSHIFT,
	KERNEL_KSC_LCTRL,
	KERNEL_KSC_RCTRL,
	KERNEL_KSC_SPACE,
	/* KERNEL_KSC_RETURN, */
	KERNEL_KSC_BACKSPACE,
};

/* Here we store the menu and the option that was selected when
 * we went to CREDITS, DEMO or HELP, to return back at the same pos.
 */
static int s_last_menu_op = -1;
static struct menu *s_last_menu;

static char s_menu_top_a[TE_FMW + 1];
static char s_menu_top_b[NELEMS(s_menu_top_a)];
static char s_menu_bottom[NELEMS(s_menu_top_a)];

static void set_redefine_state(void);
static void set_lang_state(void);

/* Loads the defined keys from preferences and redefines them. */
void load_defined_keys(void)
{
	int i, ksc;
	const char *valstr;

	for (i = 0; i < NELEMS(s_redefine_info); i++) {
		valstr = get_preference(s_redefine_info[i].pref_name);
		if (sscanf(valstr, "%d", &ksc) != 1)
			continue;
		redefine_key(s_redefine_info[i].game_key, ksc);
	}
}

/* Sets in preferences the current defined keys. */
void save_defined_keys(void)
{
	int i, ksc;

	for (i = 0; i < NELEMS(s_redefine_info); i++) {
		ksc = get_game_key_value(s_redefine_info[i].game_key);
		set_preference_int(s_redefine_info[i].pref_name, ksc);
	}
	save_prefs();
}

static int is_redefine_allowed_key(int ksc)
{
	int i;

	for (i = 0; i < NELEMS(s_allowed_keys); i++) {
		if (s_allowed_keys[i] == ksc)
			return 1;
	}

	return 0;
}

static void clear_hint(void)
{
	te_fill_fg(0, PRESS_KEY_Y - 1, TE_FMW, 2, 0, chri(' '));
}

static void draw_hint(void)
{
	int x;
	const char *str;

	str = "USE i j OR 'SPACE'";
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, PRESS_KEY_Y - 1, 0);
	str = "'ENTER' TO SELECT";
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, PRESS_KEY_Y, 0);
}

static void draw_hint_first_time(void)
{
	static int hint_drawn = 0;

	if (!hint_drawn) {
		hint_drawn = 1;
		draw_hint();
	}
}

static void draw_menu_text(const char *s, int y)
{
	int x;
	const char *str;

	te_fill_fg(0, y, TE_FMW, 1, 0, chri(' '));
	str = _(s);
	x = (TE_FMW - utf8_strlen(str)) / 2;
	draw_str(str, x, y, 0);
}

static void draw_bottom(void)
{
	char s[TE_FMW + 1];
	const char *str;
	int x;

	str = _(s_menu_bottom);
	snprintf(s, sizeof(s), "V%s %s", PACKAGE_VERSION, str);
	te_fill_fg(0, TE_FMH - 1, TE_FMW, 1, 0, chri(' '));
	x = (TE_FMW - utf8_strlen(s)) / 2;
	draw_str(s, x, TE_FMH - 1, 0);
}

static void draw_demo(void)
{
	int x;

	x = (TE_FMW - strlen("DEMO VERSION!")) / 2;
	draw_str("DEMO VERSION!", x, TE_FMH - 2, 0);
}

static void clear_demo(void)
{
	te_fill_fg(0, TE_FMH - 2, TE_FMW, 1, 0, chri(' '));
}

static void blink_demo(void)
{
	enum {
		BLINK_DEMO_TIME = FPS * 2
	};

	static int t = 0;
	static int showing = 0;

	t = dectime(t);
	if (t <= 0) {
		t = BLINK_DEMO_TIME;
		if (initfile_getvar("demo_version") != 0) {
			if (!showing) {
				draw_demo();
			} else {
				clear_demo();
			}
		}
		showing ^= 1;
	}
}

static void draw_top_bottom(void)
{
	draw_menu_text(s_menu_top_a, 0);
	draw_menu_text(s_menu_top_b, 1);
	draw_bottom();
}

static void draw(void)
{
	te_begin_draw();
	te_draw();
	te_end_draw();
}

static void set_menu_music(void)
{
	if (is_music_enabled()) {
		s_options_menu.options[OP_MUSIC_INDEX].str = TX_MUSIC_OFF;
	} else {
		s_options_menu.options[OP_MUSIC_INDEX].str = TX_MUSIC_ON;
	}
}

static void push_options_menu(int firstop)
{
	set_menu_music();
	menu_push(&s_options_menu, OPTIONS_MENU_Y, firstop,
		  s_options_menu_add);
}

static void toggle_music(void)
{
	if (is_music_enabled()) {
		set_preference("music", "0");
	} else {
		set_preference("music", "1");
	}
	save_prefs();
	set_menu_music();
	menu_redraw();
	if (is_music_enabled()) {
		modplay_play(1);
	} else {
		modplay_stop();
	}
}

static void update_main_menu(void)
{
	const struct kernel_device *d;
	int r;

	d = kernel_get_device();
	r = menu_update();
	switch (r) {
	case OP_PLAY:
		clear_hint();
	       	mixer_play(wav_play);
	       	fade_to_state(&gplay_st);
		break;
	case OP_OPTIONS:
		clear_hint();
		mixer_play(wav_opsel);
		push_options_menu(-1);
		break;
	case OP_LANG:
		clear_hint();
		mixer_play(wav_opsel);
	       	set_lang_state();
	       	break;
	case OP_HELP:
		s_last_menu_op = OP_HELP;
		s_last_menu = &s_main_menu;
		clear_hint();
		mixer_play(wav_opsel);
		free_actor(get_actor(AC_MENU));
		scroll_to_map(initfile_getvar("help_map"));
		break;
	case OP_EXIT:
		end_state();
	       	break;
	case OP_REDEFINE:
		mixer_play(wav_opsel);
	       	set_redefine_state();
		break;
	case OP_MUSIC:
		mixer_play(wav_opsel);
	       	toggle_music();
		break;
	case OP_DEMO:
		s_last_menu_op = OP_DEMO;
		s_last_menu = &s_options_menu;
		clear_hint();
	       	mixer_play(wav_play);
		if (initfile_getvar("demo") == 1) {
			fade_to_state(&demo_st);
		}
		break;
	case OP_CREDITS:
		s_last_menu_op = OP_CREDITS;
		s_last_menu = &s_options_menu;
		clear_hint();
		mixer_play(wav_opsel);
		free_actor(get_actor(AC_MENU));
		scroll_to_map(initfile_getvar("credits_map"));
		break;
	case OP_BACK:
		mixer_play(wav_opsel);
		menu_pop();
		break;
	case -2: 
		mixer_play(wav_opmove);
		break;
	case -3:
		if (menu_stack_count() > 1) {
			mixer_play(wav_opsel);
			menu_pop();
		} else {
			end_state();
		}
	case -1:
		if (s_cheats_debug_mode &&
			d->key_first_pressed(KERNEL_KSC_R))
	       	{
			load_map(initfile_getvar("menu_map"));
		}
	}
}

static void clear_press_key(void)
{
	te_fill_fg(0, PRESS_KEY_Y, TE_FMW, 1, 0, chri(' '));
}

static void draw_press_key(void)
{
	int x;
	const char *str;

	clear_press_key();
	str = s_redefine_info[s_redefine_keyi].str;
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, PRESS_KEY_Y, 0);
}

static int is_redefined_key(int ksc)
{
	int i;

	for (i = 0; i < s_redefine_keyi; i++) {
		if (ksc == s_redefined_keys[i])
			return 1;
	}

	return 0;
}

static void update_redefine(void)
{
	int ksc;
	const struct kernel_device *kdev;

	kdev = kernel_get_device();
	for (ksc = 0; ksc < KERNEL_KSC_PAD0_A; ksc++) {
		if (is_redefine_allowed_key(ksc) &&
			!is_redefined_key(ksc) &&
			kdev->key_first_pressed(ksc))
	       	{
			mixer_play(wav_opmove);
			redefine_key(s_redefine_info[s_redefine_keyi].game_key,
				     ksc);
			s_redefined_keys[s_redefine_keyi] = ksc;
			s_redefine_keyi++;
			if (s_redefine_keyi == NELEMS(s_redefine_info)) {
				save_defined_keys();
				clear_press_key();
				menu_enable(1);
				s_state = STATE_MAIN_MENU;
				break;
			} else {
				draw_press_key();
			}
		}
	}
}

static void set_redefine_state(void)
{
	s_redefine_keyi = 0;
	menu_enable(0);
	draw_press_key();
	s_state = STATE_REDEFINE;
}

static void draw_cur_lang(void)
{
	int i, y, x;
	const char *name;
	int len;

	i = get_cur_lang_index();
	name = get_lang_name(i);
	len = utf8_strlen(name);
	y = menu_get_cur_op_y();
	x = menu_get_x();
	te_fill_fg(0, y, TE_FMW, 1, 0, chri(' '));
	draw_str(name, x, y, 0);
	te_set_fg_xy(x - 2, y, 0, chri(CHR_ARROW_L)); 
	te_set_fg_xy(x + len + 1, y, 0, chri(CHR_ARROW_R)); 
}

static void update_lang(void)
{
	int i;

	if (is_first_pressed(LKLEFT) || is_first_pressed(LKUP)) {
		mixer_play(wav_opmove);
		i = get_cur_lang_index();
		if (i == 0)
			i = get_nlangs() - 1;
		else
			i--;
		load_lang(get_lang_code(i));
		draw_cur_lang();
	} else if (is_first_pressed(LKRIGHT) || is_first_pressed(LKDOWN) ||
		kernel_get_device()->key_first_pressed(KERNEL_KSC_SPACE))
       	{
		mixer_play(wav_opmove);
		i = get_cur_lang_index();
		if (i == get_nlangs() - 1)
			i = 0;
		else
			i++;
		load_lang(get_lang_code(i));
		draw_cur_lang();
	} else if (is_first_pressed(LKEYA)) {
		mixer_play(wav_opsel);

		i = get_cur_lang_index();
		set_preference("lang", get_lang_code(i));
		save_prefs();

		i = menu_get_cur_op_i();
		menu_pop();
		menu_push(&s_main_menu, MAIN_MENU_Y, i, NULL);
		draw_top_bottom();

		s_state = STATE_MAIN_MENU;
	}
}

/* Language selection */
static void set_lang_state(void)
{
	menu_enable(0);
	draw_cur_lang();
	kernel_get_device()->clear_first_pressed_keys();
	s_state = STATE_LANG;
}

static void update(void)
{
	exec_update_fns();
	update_actors();
	exec_init_map_code();

	switch (s_state) {
	case STATE_REDEFINE: update_redefine(); break;
	case STATE_LANG: update_lang(); break;
	}
}

static void update_menu_actor(struct actor *pac)
{
	blink_demo();
	if (s_state == STATE_MAIN_MENU)
		update_main_menu();
}

static void enter(const struct state *old_state)
{
	load_map(initfile_getvar("menu_map"));
	draw_hint_first_time();
	s_state = STATE_MAIN_MENU;
}

static void leave(const struct state *new_state)
{
	modplay_stop();
}

static void end(void)
{
	/* save_prefs(); */
	kernel_get_device()->stop();
}

const struct state menu_st = {
	.update = update,
	.draw = draw,
	.enter = enter,
	.leave = leave,
	.end = end
};

/* Returns the new position in s. */
static const char *extract_menu_text(const char *s, char *dst, int dstsz)
{
	int i;

	for (i = 0; *s != '\0' && *s != '|'; s++, i++) {
		if (i < dstsz - 1)
			*dst++ = *s;
	}
	if (dstsz > 0)
		*dst = '\0';
	if (*s == '|')
		s++;
	return s;
}

/* Extracts from a string 'first|second|bottom'. */
static void extract_menu_top_bottom(const char *s)
{
	s = extract_menu_text(s, s_menu_top_a, NELEMS(s_menu_top_a));
	s = extract_menu_text(s, s_menu_top_b, NELEMS(s_menu_top_b));
	extract_menu_text(s, s_menu_bottom, NELEMS(s_menu_bottom));
}

static struct actor *spawn_menu_fp(int x, int y)
{
	struct actor *pac;
	int r;
	char str[256];

	/* This contains the top and bottom texts. */
	r = tokscanf(NULL, "S", str, NELEMS(str));
	if (r != 1) {
		str[0] = '\0';
	}

	if (initfile_getvar("demo") == 0) {
		s_options_menu = s_options_menu_nodemo;
	}

	extract_menu_top_bottom(str);

	pac = get_actor(AC_MENU);
	pac->update = update_menu_actor;
	draw_top_bottom();
	menu_reset_stack();
	if (s_last_menu == &s_options_menu) {
		menu_push(&s_main_menu, MAIN_MENU_Y, OP_OPTIONS, NULL);
		push_options_menu(s_last_menu_op);
	} else {
		menu_push(&s_main_menu, MAIN_MENU_Y, s_last_menu_op, NULL);
	}
	s_last_menu = NULL;
	s_last_menu_op = -1;

	spawn_cheats();

	return pac;
}

/* Must init after input and prefs */
void menu_st_init(void)
{
	int i;

	register_spawn_fn("menu", spawn_menu_fp);
	register_sound(&wav_play, "opplay");

	/* Set initial keys */
	for (i = 0; i < NELEMS(s_redefine_info); i++) {
		set_preference_int(s_redefine_info[i].pref_name,
			get_game_key_value(s_redefine_info[i].game_key));
	}
}
