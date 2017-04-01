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

#include "menu_st.h"
#include "game.h"
#include "input.h"
#include "text.h"
#include "tilengin.h"
#include "data.h"
#include "fade_st.h"
#include "load_st.h"
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
#include "hiscore.h"
#include "google.h"
#include "coroutin.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#include "cfg/cfg.h"
#include <string.h>

static struct wav *wav_play;

#define TX_PLAY		"PLAY"
#define TX_OPTIONS	"OPTIONS"
#define TX_LANG		"LANGUAGE"
#define TX_HELP		"HELP"
#define TX_EXIT		"QUIT"

#define TX_REDEFINE	"DEFINE KEYS"
#define TX_SOUND	"SOUND"
#define TX_DEMO		"DEMO"
#define TX_CREDITS	"CREDITS"
#define TX_LEADERBOARDS	"LEADERBOARDS"
#define TX_ACHIEVEMENTS	"ACHIEVEMENTS"
#define TX_GOOGLE	"GOOGLE PLAY"
#define TX_BACK		"BACK"

#define TX_BEGINNER	"BEGINNER"
#define TX_EXPERT	"EXPERT"

#define TX_ON		"ON"
#define TX_OFF		"OFF"
#define TX_VOLUME	"VOLUME"
#define TX_MUSIC_ON	"MUSIC: ON"
#define TX_MUSIC_OFF	"MUSIC: OFF"
#define TX_VOLUME_100	"VOLUME: 100"

#define TX_CONNECT_Q	"DO YOU WANT TO SIGN IN WITH GOOGLE TO ENTER ONLINE LEADERBOARDS AND UNLOCK ACHIEVEMENTS?"

#define TX_RETRY_Q	"THERE WAS AN ERROR SIGNING IN WITH GOOGLE. RETRY?"

static int ask_connect_then_play_menu(int restart);
static int connect_then_play_menu(int restart);
static int ask_connect_then_google(int restart);
static int connect_then_google(int restart);
static int ask_connect_then_leaderboard(int restart);
static int connect_then_leaderboard(int restart);
static int show_leaderboard_cr(int restart);
static int ask_connect_then_achievements(int restart);
static int connect_then_achievements(int restart);
static int show_achievements_cr(int restart);
static int autoconnect_failed_cr(int restart);

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
	OP_BEGINNER,
	OP_EXPERT,
	OP_SOUND,
	OP_VOLUME,
	OP_GOOGLE,
	OP_LEADERBOARDS,
	OP_LEADERBOARDS_BEGINNER,
	OP_LEADERBOARDS_EXPERT,
	OP_ACHIEVEMENTS,
};

enum {
	MAIN_MENU_Y = 13,
	OPTIONS_MENU_Y = 13,
	LEVEL_MENU_Y = 15,
	SOUND_MENU_Y = 15,
	LEADERBOARDS_MENU_Y = 15,
	GOOGLE_MENU_Y = 15,
	OP_MUSIC_INDEX = 0,
	OP_VOLUME_INDEX = 1,
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
#if PP_PHONE_MODE
		{ TX_GOOGLE, OP_GOOGLE, "pqrs" },
#else
		{ TX_REDEFINE, OP_REDEFINE },
#endif
		{ TX_SOUND, OP_SOUND },
		{ TX_CREDITS, OP_CREDITS },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static struct menu s_options_menu = {
	.options = {
#if PP_PHONE_MODE
		{ TX_GOOGLE, OP_GOOGLE, "pqrs" },
#else
		{ TX_REDEFINE, OP_REDEFINE },
#endif
		{ TX_SOUND, OP_SOUND },
		{ TX_DEMO, OP_DEMO },
		{ TX_CREDITS, OP_CREDITS },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static char s_volume_str[32] = TX_VOLUME;

static struct menu s_sound_menu = {
	.options = {
		{ TX_MUSIC_ON, OP_MUSIC },
		{ s_volume_str, OP_VOLUME },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static const char *s_sound_menu_add[] = {
	TX_MUSIC_ON,
	TX_MUSIC_OFF,
	TX_VOLUME_100,
	NULL
};

static struct menu s_level_menu = {
	.options = {
		{ TX_BEGINNER, OP_BEGINNER },
		{ TX_EXPERT, OP_EXPERT },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static struct menu s_google_menu = {
	.options = {
		{ TX_LEADERBOARDS, OP_LEADERBOARDS },
		{ TX_ACHIEVEMENTS, OP_ACHIEVEMENTS },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

static struct menu s_leaderboards_menu = {
	.options = {
		{ TX_BEGINNER, OP_LEADERBOARDS_BEGINNER },
		{ TX_EXPERT, OP_LEADERBOARDS_EXPERT },
		{ TX_BACK, OP_BACK },
		{ NULL, -2 },
	},
};

enum {
	OP_YES,
	OP_NO,
};

enum {
	STATE_MAIN_MENU,
	STATE_REDEFINE,
	STATE_LANG,
	STATE_VOLUME,
};

static int s_state;

enum {
	PRESS_KEY_Y = TE_FMH - 4,
};

static int s_hint_drawn;

static int s_redefine_keyi;

struct redefine_info {
	const char *str;
	const char *pref_name;
	int game_key;
};

static const struct redefine_info s_redefine_info[] = {
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
static void set_volume_state(void);
static void connection_question_answered(int accepted);

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

	if (PP_PHONE_MODE) {
		str = "USE i OR j";
	} else {
		str = "USE i j OR 'SPACE'";
	}
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, PRESS_KEY_Y - 1, 0);
	if (PP_PHONE_MODE) {
		str = "'A' TO SELECT";
	} else {
		str = "'ENTER' TO SELECT";
	}
	x = (TE_FMW - utf8_strlen(_(str))) / 2;
	draw_str(_(str), x, PRESS_KEY_Y, 0);
}

static void draw_hint_first_time(void)
{

	if (!s_hint_drawn) {
		s_hint_drawn = 1;
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
	te_fill_fg(2, TE_FMH - 1, TE_FMW - 4, 1, 0, chri(' '));
	x = (TE_FMW - utf8_strlen(s)) / 2;
	draw_str(s, x, TE_FMH - 1, 0);
}

static void draw_demo(void)
{
	int x;
	const char *str;

	str = _("DEMO VERSION");
	x = (TE_FMW - utf8_strlen(str)) / 2;
	draw_str(str, x, TE_FMH - 2, 0);
}

static void clear_demo(void)
{
	te_fill_fg(2, TE_FMH - 2, TE_FMW - 4, 1, 0, chri(' '));
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

static void set_google_icon(void)
{
	if (!PP_PHONE_MODE) {
		return;
	}

	if (Android_IsConnectedToGooglePlay()) {
		draw_icon("pqrs", TE_FMW - 2, TE_FMH - 2, 0);
		// s_main_menu.options[0].icon = "pqrs";
		s_options_menu.options[0].icon = "pqrs";
	} else {
		draw_icon("ptru", TE_FMW - 2, TE_FMH - 2, 0);
		// s_main_menu.options[0].icon = "ptru";
		s_options_menu.options[0].icon = "ptru";
	}
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
		s_sound_menu.options[OP_MUSIC_INDEX].str = TX_MUSIC_ON;
	} else {
		s_sound_menu.options[OP_MUSIC_INDEX].str = TX_MUSIC_OFF;
	}
}

static void set_menu_volume(void)
{
	snprintf(s_volume_str, sizeof(s_volume_str), "%s: %d", _(TX_VOLUME),
		 mixer_get_volume());
}

static void push_sound_menu(int op)
{
	set_menu_music();
	set_menu_volume();
	menu_push(&s_sound_menu, SOUND_MENU_Y, op, s_sound_menu_add);
}

static void push_options_menu(int firstop)
{
	menu_push(&s_options_menu, OPTIONS_MENU_Y, firstop, NULL);
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

static void push_main_menu(int firstop)
{
	menu_push(&s_main_menu, MAIN_MENU_Y, firstop, NULL);
}

static void push_level_menu(int firstop)
{
	menu_push(&s_level_menu, LEVEL_MENU_Y, firstop, NULL);
}

static void leaderboards_selected(void)
{
	menu_push(&s_leaderboards_menu, LEADERBOARDS_MENU_Y, -1, NULL);
}

static void push_google_menu(void)
{
	menu_push(&s_google_menu, GOOGLE_MENU_Y, -1, NULL);
}

static void google_selected(void)
{
	if (Android_IsConnectedToGooglePlay()) {
		push_google_menu();
	} else {
		start_coroutine(ask_connect_then_google);
	}
}

static void try_show_leaderboard(void)
{
	if (Android_IsConnectedToGooglePlay()) {
		start_coroutine(show_leaderboard_cr);
	} else {
		start_coroutine(ask_connect_then_leaderboard);
	}
}

static void try_show_achievements(void)
{
	if (Android_IsConnectedToGooglePlay()) {
		start_coroutine(show_achievements_cr);
	} else {
		start_coroutine(ask_connect_then_achievements);
	}
}

static void connection_question_answered(int accepted)
{
	set_preference_int("connectq", 1);
	set_preference_int("autoconnect", accepted != 0);
	save_prefs();
}

static void try_play_menu(void)
{
	if (PP_PHONE_MODE &&
		!preference_equals("connectq", "1") &&
	       	!Android_IsConnectedToGooglePlay())
       	{
		start_coroutine(ask_connect_then_play_menu);
	} else {
		push_level_menu(-1);
	}
}

static void update_main_menu(void)
{
	const struct kernel_device *d;
	int r;

	if (is_coroutine_running()) {
		run_coroutine();
		return;
	}

	d = kernel_get_device();
	r = menu_update();
	switch (r) {
	case OP_PLAY:
		clear_hint();
		mixer_play(wav_opsel);
		try_play_menu();
		break;
	case OP_BEGINNER:
		set_difficulty(DIFFICULTY_BEGINNER);
	       	mixer_play(wav_play);
	       	fade_to_state(&gplay_st);
		break;
	case OP_EXPERT:
		set_difficulty(DIFFICULTY_EXPERT);
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
	case OP_SOUND:
		mixer_play(wav_opsel);
		push_sound_menu(-1);
		break;
	case OP_DEMO:
		set_difficulty(DIFFICULTY_EXPERT);
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
	case OP_MUSIC:
		mixer_play(wav_opsel);
	       	toggle_music();
		break;
	case OP_VOLUME:
		mixer_play(wav_opsel);
	       	set_volume_state();
		break;
	case OP_GOOGLE:
		mixer_play(wav_opsel);
		google_selected();
		break;
	case OP_LEADERBOARDS:
		mixer_play(wav_opsel);
		leaderboards_selected();
		break;
	case OP_LEADERBOARDS_BEGINNER:
		mixer_play(wav_opsel);
		set_difficulty(DIFFICULTY_BEGINNER);
		try_show_leaderboard();
		break;
	case OP_LEADERBOARDS_EXPERT:
		mixer_play(wav_opsel);
		set_difficulty(DIFFICULTY_EXPERT);
		try_show_leaderboard();
		break;
	case OP_ACHIEVEMENTS:
		mixer_play(wav_opsel);
		try_show_achievements();
		break;
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

		menu_pop();
		push_main_menu(OP_LANG);
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

static void draw_cur_volume(void)
{
	int y, x;
	int len;
	char str[4];

	snprintf(str, sizeof(str), "%d", mixer_get_volume());
	len = utf8_strlen(str);
	y = menu_get_cur_op_y();
	x = menu_get_x();
	te_fill_fg(0, y, TE_FMW, 1, 0, chri(' '));
	draw_str(str, x, y, 0);
	te_set_fg_xy(x - 2, y, 0, chri(CHR_ARROW_L)); 
	te_set_fg_xy(x + len + 1, y, 0, chri(CHR_ARROW_R)); 
}

static void update_volume(void)
{
	int vol;

	vol = mixer_get_volume();
	if (is_first_pressed(LKLEFT) || is_first_pressed(LKUP)) {
		mixer_play(wav_opmove);
		if (vol == 0) {
			mixer_set_volume(100);
		} else {
			mixer_set_volume(vol - 10);
		}
		draw_cur_volume();
	} else if (is_first_pressed(LKRIGHT) || is_first_pressed(LKDOWN) ||
		kernel_get_device()->key_first_pressed(KERNEL_KSC_SPACE))
       	{
		mixer_play(wav_opmove);
		if (vol == 100) {
			mixer_set_volume(0);
		} else {
			mixer_set_volume(vol + 10);
		}
		draw_cur_volume();
	} else if (is_first_pressed(LKEYA)) {
		mixer_play(wav_opsel);

		set_preference_int("volume", mixer_get_volume());
		save_prefs();
		
		menu_pop();
		push_sound_menu(OP_VOLUME);

		s_state = STATE_MAIN_MENU;
	}
}

/* Volume selection */
static void set_volume_state(void)
{
	menu_enable(0);
	draw_cur_volume();
	kernel_get_device()->clear_first_pressed_keys();
	s_state = STATE_VOLUME;
}

static void update(void)
{
	exec_update_fns();
	update_actors();
	exec_init_map_code();

	switch (s_state) {
	case STATE_REDEFINE: update_redefine(); break;
	case STATE_LANG: update_lang(); break;
	case STATE_VOLUME: update_volume(); break;
	}
}

static void update_menu_actor(struct actor *pac)
{
	blink_demo();
	if (s_state == STATE_MAIN_MENU) {
		update_main_menu();
	}
}

static void enter(const struct state *old_state)
{
	load_map(initfile_getvar("menu_map"));
	draw_hint_first_time();
	s_state = STATE_MAIN_MENU;

	if (s_google_autoconnect_performed &&
	    !Android_IsConnectedToGooglePlay())
       	{
		s_google_autoconnect_performed = 0;
		start_coroutine(autoconnect_failed_cr);
	}
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
	set_google_icon();
	menu_reset_stack();
	if (s_last_menu == &s_options_menu) {
		push_main_menu(OP_OPTIONS);
		push_options_menu(s_last_menu_op);
	} else {
		push_main_menu(s_last_menu_op);
	}
	s_last_menu = NULL;
	s_last_menu_op = -1;

	spawn_cheats();

	return pac;
}

static void show_connect_msgbox(void)
{
	static const struct msgbox mb = {
		.icon = "ptru",
		.title = TX_CONNECT_Q,
		.options = { 
			{ "YES", OP_YES },
			{ "NO" , OP_NO},
		},
		.x = 3,
		.w = TE_FMW - 3 * 2,
		.can_go_back = 1
	};

	show_msgbox(&mb);
}

static void show_retry_msgbox(void)
{
	static const struct msgbox mb = {
		.icon = "ptru",
		.title = TX_RETRY_Q,
		.options = { 
			{ "YES", OP_YES },
			{ "NO", OP_NO },
		},
		.x = 3,
		.w = TE_FMW - 3 * 2,
		.can_go_back = 0
	};

	show_msgbox(&mb);
}

static void show_error_info_msgbox(void)
{
	static const struct msgbox mb = {
		.icon = "pqrs",
		.title = "IF YOU WANT TO SIGN IN IN THE FUTURE, GO TO 'OPTIONS - GOOGLE PLAY'.",
		.options = { 
			{ "OK", OP_YES },
		},
		.x = 3,
		.w = TE_FMW - 3 * 2,
		.can_go_back = 0
	};

	show_msgbox(&mb);
}

#define UPDATE_MSGBOX \
	do { \
		r = update_msgbox(); \
		while (r == MSGBOX_IDLE) { \
			crReturn(0); \
			r = update_msgbox(); \
		} \
	} while (0)

/* Ask and connect to google play */
static int ask_connect_then_play_menu(int restart)
{
	int r;

	crBegin(restart);
	show_connect_msgbox();
	crReturn(0);
	UPDATE_MSGBOX;
	if (r == OP_YES) {
		start_coroutine(connect_then_play_menu);
		crReturn(1);
	} else {
		show_error_info_msgbox();
		crReturn(0);
		UPDATE_MSGBOX;
		connection_question_answered(0);
	}
	crFinish;
	push_level_menu(-1);
	return 1;
}

/* Connect to google play */
static int connect_then_play_menu(int restart)
{
	int r;

	crBegin(restart);
connect:	
	Android_ConnectToGooglePlay();
	while (Android_IsConnectingToGooglePlay()) {
		crReturn(0);
	}
	if (!Android_IsConnectedToGooglePlay()) {
		show_retry_msgbox();
		crReturn(0);
		UPDATE_MSGBOX;
		if (r == OP_YES) {
			crReturn(0);
			goto connect;
		} else {
			show_error_info_msgbox();
			crReturn(0);
			UPDATE_MSGBOX;
			connection_question_answered(0);
		}
	} else {
		set_google_icon();
		connection_question_answered(1);
	}
	crFinish;
	push_level_menu(-1);
	return 1;
}

static int ask_connect_then_google(int restart)
{
	int r;

	crBegin(restart);
	show_connect_msgbox();
	crReturn(0);
	UPDATE_MSGBOX;
	if (r == OP_YES) {
		start_coroutine(connect_then_google);
		crReturn(1);
	} else {
		connection_question_answered(0);
	}
	crFinish;
	return 1;
}

static int connect_then_google(int restart)
{
	int r;

	crBegin(restart);
connect:	
	Android_ConnectToGooglePlay();
	while (Android_IsConnectingToGooglePlay()) {
		crReturn(0);
	}
	if (!Android_IsConnectedToGooglePlay()) {
		show_retry_msgbox();
		crReturn(0);
		UPDATE_MSGBOX;
		if (r == OP_YES) {
			crReturn(0);
			goto connect;
		} else {
			connection_question_answered(0);
		}
	} else {
		set_google_icon();
		connection_question_answered(1);
		push_google_menu();
	}
	crFinish;
	return 1;
}

static int ask_connect_then_leaderboard(int restart)
{
	int r;

	crBegin(restart);
	show_connect_msgbox();
	crReturn(0);
	UPDATE_MSGBOX;
	if (r == OP_YES) {
		start_coroutine(connect_then_leaderboard);
		crReturn(1);
	} else {
		connection_question_answered(0);
	}
	crFinish;
	return 1;
}

static int connect_then_leaderboard(int restart)
{
	int r;

	crBegin(restart);
connect:	
	Android_ConnectToGooglePlay();
	while (Android_IsConnectingToGooglePlay()) {
		crReturn(0);
	}
	if (!Android_IsConnectedToGooglePlay()) {
		show_retry_msgbox();
		crReturn(0);
		UPDATE_MSGBOX;
		if (r == OP_YES) {
			crReturn(0);
			goto connect;
		} else {
			connection_question_answered(0);
		}
	} else {
		set_google_icon();
		connection_question_answered(1);
		start_coroutine(show_leaderboard_cr);
	}
	crFinish;
	return 1;
}

static int show_leaderboard_cr(int restart)
{
	crBegin(restart);
	Android_ShowLeaderboard(s_board_id[get_difficulty()]);
	while (Android_IsRequestingLeaderboard()) {
		crReturn(0);
	}
	if (!Android_IsConnectedToGooglePlay()) {
		set_google_icon();
		set_preference_int("autoconnect", 0);
		save_prefs();
	}
	crFinish;
	return 1;
}

static int show_achievements_cr(int restart)
{
	crBegin(restart);
	Android_ShowAchievements();
	while (Android_IsRequestingAchievements()) {
		crReturn(0);
	}
	if (!Android_IsConnectedToGooglePlay()) {
		set_google_icon();
		set_preference_int("autoconnect", 0);
		save_prefs();
	}
	crFinish;
	return 1;
}

static int ask_connect_then_achievements(int restart)
{
	int r;

	crBegin(restart);
	show_connect_msgbox();
	crReturn(0);
	UPDATE_MSGBOX;
	if (r == OP_YES) {
		start_coroutine(connect_then_achievements);
		crReturn(1);
	} else {
		connection_question_answered(0);
	}
	crFinish;
	return 1;
}

static int connect_then_achievements(int restart)
{
	int r;

	crBegin(restart);
connect:	
	Android_ConnectToGooglePlay();
	while (Android_IsConnectingToGooglePlay()) {
		crReturn(0);
	}
	if (!Android_IsConnectedToGooglePlay()) {
		show_retry_msgbox();
		crReturn(0);
		UPDATE_MSGBOX;
		if (r == OP_YES) {
			crReturn(0);
			goto connect;
		} else {
			connection_question_answered(0);
		}
	} else {
		set_google_icon();
		connection_question_answered(1);
		start_coroutine(show_achievements_cr);
	}
	crFinish;
	return 1;
}

static int autoconnect_failed_cr(int restart)
{
	int r;

	crBegin(restart);
retry:  show_retry_msgbox();
	crReturn(0);
	UPDATE_MSGBOX;
	if (r == OP_YES) {
		crReturn(0);
		Android_ConnectToGooglePlay();
		while (Android_IsConnectingToGooglePlay()) {
			crReturn(0);
		}
		if (!Android_IsConnectedToGooglePlay()) {
			goto retry;
		}
	} else {
		show_error_info_msgbox();
		crReturn(0);
		UPDATE_MSGBOX;
		set_preference_int("autoconnect", 0);
		save_prefs();
	}
	crFinish;
	set_google_icon();
	return 1;
}

/* Must init after input and prefs */
void menu_st_init(void)
{
	int i;

	s_hint_drawn = 0;
	register_spawn_fn("menu", spawn_menu_fp);
	register_sound(&wav_play, "opplay");

	/* Set initial keys */
	for (i = 0; i < NELEMS(s_redefine_info); i++) {
		set_preference_int(s_redefine_info[i].pref_name,
			get_game_key_value(s_redefine_info[i].game_key));
	}
}
