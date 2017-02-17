/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "game.h"
#include "tilengin.h"
#include "data.h"
#include "load_st.h"
#include "cosmonau.h"
#include "oxigen.h"
#include "stargate.h"
#include "hud.h"
#include "debug.h"
#include "cheats.h"
#include "input.h"
#include "text.h"
#include "strdraw.h"
#include "fade.h"
#include "modplay.h"
#include "msgbox.h"
#include "prefs.h"
#include "demo_st.h"
#include "hiscore.h"
#include "pad.h"
#include "gamelib/vfs.h"
#include "gamelib/mixer.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"
#ifndef CONFIG_H
#define CONFIG_H
#include "config.h"
#endif
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <locale.h>

enum {
	NSPAWN_FNS = 32,
	NMAP_INIT_FNS = 8,
	NUPDATE_FNS = 8,
	CODE_TEXT_CHAR_DELAY = 4,
	CODE_SCROLL_DELAY = 4,
};

/* DIFFICULTY_NORMAL, EASY, etc. */
int s_difficulty;

/* If we go step by step. */
static int s_step_mode;

static struct actor *s_last_spawned_actor;
static int s_last_created_path;
static int s_map_id = -1;
static int s_scheduled_map_exit = -1;
static unsigned char s_map_exits[4];

/* Backup state when entering room. */
static int s_exit_side_backup = -1;
static struct actor s_cosmo_actor_backup;
static struct sprite s_cosmo_sprite_backup;

/* Map id, stargate id scheduled to teleport to. */
static int s_teleport_mapid = -1;
static int s_teleport_gateid = -1;
static int s_teleport_next_active_gateid = -1;

/* Win condition scheduled. */
static int s_win_scheduled = 0;

struct spawn_info {
	const char *id;
	spawn_pfn_t fn;
};

/* 
 * Index in s_spawn_table of the first item that can be filled by
 * register_spawn_fn() .
 */
static int s_free_spawnfn_index;
static struct spawn_info s_spawn_table[NSPAWN_FNS];

/* 
 * Index in s_map_init_table of the first item that can be filled by
 * register_map_init_fn() .
 */
static int s_free_mapinitfn_index;
static map_init_pfn_t s_map_init_table[NMAP_INIT_FNS];

/* 
 * Index in s_update_table of the first item that can be filled by
 * register_update_fn() .
 */
static int s_free_updatefn_index;
static update_pfn_t s_update_table[NUPDATE_FNS];

/* the actor table. */
static struct actor s_actors[NACTORS];

struct instr {
	char cmdid[64];
	char args[256];
};

static struct instr s_code[128];
static int s_code_pc;
static void (*s_code_fn)(void) = NULL;
static int s_code_t;
static int s_code_text_x;
static int s_code_text_x_i;
static int s_code_text_y;
static int s_code_text_sub_i;
static int s_code_text_char_i;
static char s_code_text[256];
static int s_code_scroll_x;
static int s_code_scroll_mapid;
static struct te_map_info s_code_scroll_map_info;

/*
 * Returns a pointer to an internal string with the name of
 * the map file for an id.
 * For example, for id 0, returns "r00.txt".
 */
static const char *get_map_fname(int id)
{
	static char fname[16];

	snprintf(fname, sizeof(fname), "r%02x.txt", id);
	return fname;
}

/*
 * Returns a pointer to an internal string with the name of
 * the map init file for an id.
 * For example, for id 0, returns "data/i00.txt".
 */
static const char *get_map_init_fname(int id)
{
	static char fname[16];

	snprintf(fname, sizeof(fname), "data/i%02x.txt", id);
	return fname;
}

void register_spawn_fn(const char *id, spawn_pfn_t fn)
{
	struct spawn_info *psi;

	kasserta(s_free_spawnfn_index < NSPAWN_FNS);
	kasserta(fn != NULL);

	psi = &s_spawn_table[s_free_spawnfn_index++];
	psi->id = id;
	psi->fn = fn;
}

int tokscanf(char *str, const char *fmt, ...)
{
	va_list ap;
	char *s, *tok;
	size_t i, j, n;
	int r;
	int *pi;

	r = 0;
	va_start(ap, fmt);
	while (fmt != NULL && *fmt != '\0') {
		switch (*fmt++) {
		case 'S':
			/* take a string until the end */
			tok = strtok(str, "");
			s = va_arg(ap, char *);
			n = va_arg(ap, size_t);
			if (n > 0) {
				j = 0;
				if (tok != NULL) {
					for (i = 0; tok[i] != '\0' &&
						j < n - 1; i++, j++)
					{
						if (tok[i] == '\\' && 
							tok[i + 1] == 'n')
						{
							s[j] = '\n';
							i++;
						} else {
							s[j] = tok[i];
						}
					}
				}
				s[j] = '\0';
			}
			r++;
			break;
		case 's':
			/* take a word */
			tok = strtok(str, " ");
			if (tok == NULL) {
				fmt = NULL;
				break;
			}
			s = va_arg(ap, char *);
			n = va_arg(ap, size_t);
			if (n > 0) {
				if (tok != NULL) {
					for (i = 0; tok[i] != '\0' &&
						i < n - 1; i++)
					{
						s[i] = tok[i];
					}
				}
				s[i] = '\0';
			}
			r++;
			break;
		case 'i':
			tok = strtok(str, " ");
			if (tok == NULL) {
				fmt = NULL;
				break;
			}
			pi = va_arg(ap, int *);
			if (sscanf(tok, "%i", pi) == 1)
				r++;
			else
				fmt = NULL;
			break;
		default:
			fmt = NULL;
		}
		str = NULL;
	}

	va_end(ap);
	return r;
}

static void exec_spawn(char *args)
{
	int i, x, y;
	char str[64];

	s_last_spawned_actor = NULL;

	i = tokscanf(args, "s", str, NELEMS(str));
	if (i != 1) {
		return;
	}

	i = tokscanf(NULL, "ii", &x, &y);
	if (i != 2) {
		 x = 0;
		 y = 0;
	}

	for (i = 0; i < s_free_spawnfn_index; i++) {
		if (strcmp(str, s_spawn_table[i].id) == 0) {
			s_last_spawned_actor = s_spawn_table[i].fn(x, y);
			break;
		}
	}
}

static void exec_set_path(char *args)
{
	int r, path_id, path_point, speed, loop;

	r = sscanf(args, " %d %d %d %d ", &path_id, &path_point, &speed,
		&loop);
	if (r != 4) {
		return;
	}

	if (s_last_spawned_actor != NULL && path_id >= 0 &&
		path_id < num_paths() && speed >= 0)
       	{
		set_actor_path(s_last_spawned_actor, path_id, path_point,
			speed, loop);
	}
}

static void exec_add_path_point(char *args)
{
	int r, x, y;

	if (s_last_created_path < 0) {
		return;
	}
	r = sscanf(args, " %d %d ", &x, &y);
	if (r != 2) {
		return;
	}
	add_path_point(s_last_created_path, te_t2w(x), te_t2w(y));
}

static void exec_new_path(char *args)
{
	int r, x, y;

	s_last_created_path = -1;

	r = sscanf(args, " %d %d ", &x, &y);
	if (r != 2) {
		return;
	}

	s_last_created_path = new_path(te_t2w(x), te_t2w(y));
	kassert(s_last_created_path >= 0);
}

static void exec_exits(char *args)
{
	int r, left, right, up, down;

	r = sscanf(args, " %d %d %d %d ", &left, &right, &up, &down);
	if (r != 4) {
		return;
	}

	s_map_exits[0] = left != 0;
	s_map_exits[1] = right != 0;
	s_map_exits[2] = up != 0;
	s_map_exits[3] = down != 0;
}

static void update_wait_cmd(void)
{
	const struct kernel_device *d;
	struct actor *pac;

	d = kernel_get_device();
	pac = get_actor(AC_COSMONAUT);
	if (d->key_first_pressed(KERNEL_KSC_RETURN) ||
		d->key_first_pressed(KERNEL_KSC_ESC) ||
		is_first_pressed(LKEYB) ||
		is_first_pressed(LKEYX) ||
		is_first_pressed(LKEYY) ||
		(pac->update == NULL && is_first_pressed(LKEYA)))
	{
		mixer_play(wav_opmove);
		s_code_fn = NULL;
	} else if (s_cheats_debug_mode && d->key_first_pressed(KERNEL_KSC_R)) {
		load_map(s_map_id);
	}
}

static void exec_wait(char *args)
{
	s_code_fn = update_wait_cmd;
}

static void update_wait_hiscore_cmd(void)
{
	if (!hiscore_is_working())
		s_code_fn = NULL;
}

/* Waits for the hiscore enter state to finish, if any. */
static void exec_wait_hiscore(char *args)
{
	if (hiscore_is_working())
		s_code_fn = update_wait_hiscore_cmd;
	else
		s_code_fn = NULL;
}

static void update_text_cmd(void)
{
	const struct kernel_device *d;
	const char *str;

	d = kernel_get_device();
	if (d->key_first_pressed(KERNEL_KSC_RETURN) ||
		d->key_first_pressed(KERNEL_KSC_ESC) ||
		is_first_pressed(LKEYB) ||
		is_first_pressed(LKEYX) ||
		is_first_pressed(LKEYY))
	{
		mixer_play(wav_opmove);
		str = _(s_code_text);
		draw_str_ww(str, s_code_text_x, s_code_text_y, 0, -1,
			    TE_FMW - s_code_text_x - 1);
		s_code_fn = NULL;
		return;
	}

	s_code_t = dectime(s_code_t);
	if (s_code_t > 0)
		return;
	s_code_t = CODE_TEXT_CHAR_DELAY; 

	str = _(s_code_text);
	if (str[s_code_text_char_i] == '\0') {
		s_code_fn = NULL;
	} else {
		draw_str_ww(str, s_code_text_x, s_code_text_y, 0,
			    s_code_text_char_i + 1, TE_FMW - s_code_text_x - 1);
		s_code_text_char_i++;
	}
}

static void exec_clrscr(char *args)
{
	te_clear_fg(0, chri(' '));
}

static void exec_text(char *args)
{
	int r;

	r = tokscanf(args, "iiS", &s_code_text_x, &s_code_text_y,
		s_code_text, NELEMS(s_code_text));
	if (r != 3)
		return;

	s_code_text_sub_i = 0;
	s_code_text_char_i = 0;
	s_code_text_x_i = 0;
	s_code_t = CODE_TEXT_CHAR_DELAY; 
	s_code_fn = update_text_cmd;
}

static void exec_textf(char *args)
{
	int r;

	r = tokscanf(args, "iiS", &s_code_text_x, &s_code_text_y,
		s_code_text, NELEMS(s_code_text));
	if (r != 3)
		return;

	draw_str_ww(_(s_code_text), s_code_text_x, s_code_text_y, 0, -1,
		    TE_FMW - s_code_text_x - 1);
}

/* Text fast centered. */
static void exec_textfc(char *args)
{
	int r;
	int x, y, w;

	r = tokscanf(args, "iiiS", &x, &y, &w, s_code_text,
		     NELEMS(s_code_text));
	if (r != 4)
		return;

	x += (w - utf8_strlen(_(s_code_text))) / 2;
	draw_str_ww(_(s_code_text), x, y, 0, -1, w);
}

static void update_scroll_cmd(void)
{
	int i, x, y;
	struct sprite *psp;

	s_code_t = dectime(s_code_t);
	if (s_code_t > 0)
		return;
	else
		s_code_t = CODE_SCROLL_DELAY;

	for (x = 1; x < TE_BMW; x++) {
		for (y = 0; y < TE_BMH; y++) {
			te_set_bg_xy(x - 1, y, te_bg_xy(x, y));
		}
	}

	for (y = 0; y < TE_BMH; y++) {
		te_set_bg_xy(TE_BMW - 1, y,
			s_code_scroll_map_info.
				map[y * TE_BMW + s_code_scroll_x]);
	}

	/* scroll all actors */
	for (i = 0; i < TE_NSPRITES; i++) {
		psp = te_get_sprite(i);
		if (psp->pframe != NULL) {
			psp->x -= TE_VBTW;
		}
	}

	s_code_scroll_x++;
	if (s_code_scroll_x == TE_BMW) {
		s_code_fn = NULL;
		/* fix actors out of screen */
		/*
		for (i = 0; i < TE_NSPRITES; i++) {
			psp = te_get_sprite(i);
			if (psp->pframe != NULL && psp->x < 0) {
				psp->x += TE_VBTW * TE_BMW * NELEMS(s_maps);
			}
		}
		*/
		load_map(s_code_scroll_mapid);
	}
}

static void load_map_to_mem(int mapid, struct te_map_info *minfo)
{
	te_load_map_to_mem(get_map_fname(mapid), minfo);
}

void scroll_to_map(int mapid)
{
	s_code_scroll_mapid = mapid;
	te_clear_fg(0, chri(' '));
	load_map_to_mem(s_code_scroll_mapid, &s_code_scroll_map_info);
	s_code_scroll_x = 0;
	s_code_t = CODE_SCROLL_DELAY;
	s_code_fn = update_scroll_cmd;
}

static void exec_scroll(char *args)
{
	int r, mapid;

	r = sscanf(args, " %i ", &mapid); 
	if (r != 1)
		return;

	scroll_to_map(mapid);
}

static void update_halt_cmd(void)
{
}

static void exec_halt(char *args)
{
	s_code_fn = update_halt_cmd;
}

static void exec_endstate(char *args)
{
	end_state();
}

static void exec_load(char *args)
{
	int r, mapid;

	r = sscanf(args, "%i", &mapid);
	if (r != 1)
		return;

	load_map(mapid);
}

static void fadeout_cmd(void)
{
	fade_update();
	if (is_fade_over()) {
		s_code_fn = NULL;
	}
}

static void exec_fadeout(char *args)
{
	fade_out();
	s_code_fn = fadeout_cmd;
}

static void fadein_cmd(void)
{
	fade_update();
	if (is_fade_over()) {
		disable_fade();
		s_code_fn = NULL;
	}
}

static void exec_fadein(char *args)
{
	fade_in();
	s_code_fn = fadein_cmd;
}

static void exec_music(char *args)
{
	int r;
	char modname[13];

	if (g_state == &demo_st)
		return;

	r = sscanf(args, "%12s", modname);
	if (r != 1) {
		return;
	}

	if (strcmp(modname, modplay_get_modname()) != 0) {
		modplay_load(modname);
	}
	if (is_music_enabled() && !modplay_is_playing()) {
		modplay_play(1);
	}
}

/*
 * Executes the script code for a map.
 * As the code is executed in a loop, we have to be careful to not
 * execute any code if the function will be called again for example
 * because of calling exec_load().
 */
void exec_init_map_code(void)
{
	static struct init_map_info {
		const char *str;
		void (*fn)(char *args);
		int halt;
	} table[] = {
		{ "music", exec_music, 0 },
		{ "new_path", exec_new_path, 0 },
		{ "add_path_point", exec_add_path_point, 0 },
		{ "spawn", exec_spawn, 0 },
		{ "set_path", exec_set_path, 0 },
		{ "exits", exec_exits, 0 },
		{ "wait", exec_wait, 0 },
		{ "wait_hiscore", exec_wait_hiscore, 0 },
		{ "text", exec_text, 0 },
		{ "textf", exec_textf, 0 },
		{ "textfc", exec_textfc, 0 },
		{ "clrscr", exec_clrscr, 0 },
		{ "scroll", exec_scroll, 0 },
		{ "load", exec_load, 1 },
		{ "endstate", exec_endstate, 1 },
		{ "fadeout", exec_fadeout, 0 },
		{ "fadein", exec_fadein, 0 },
		{ ".", exec_halt, 0 },
	};

	int i, pc, halt;

	if (s_code_fn != NULL) {
		s_code_fn();
		return;
	}

	halt = 0;
	while (!halt && s_code_fn == NULL && s_code_pc < NELEMS(s_code)) {
		pc = s_code_pc++;
		for (i = 0; i < NELEMS(table); i++) {
			if (strcmp(s_code[pc].cmdid, table[i].str) == 0) 
			{
				halt = table[i].halt;
				table[i].fn(s_code[pc].args);
				break;
			}
		}
	}
}

/*
 * Loads the init map file for the map id "data/ixxx.txt".
 */
static void load_init_map(int id)
{
	FILE *fp;
	int i, c;
	struct instr *pinstr;

	s_code_pc = 0;
	strcpy(s_code[0].cmdid, ".");
	s_code_fn = NULL;

	if ((fp = open_file(get_map_init_fname(id), NULL)) == NULL)
		return;

	c = ' ';
	while (c != EOF && s_code_pc < NELEMS(s_code) - 1) {
		pinstr = &s_code[s_code_pc];

		fscanf(fp, " ");

		i = 0;
		for (c = getc(fp);
			c != EOF && c != '~' && !isspace(c);
			c = getc(fp))
		{
			if (i < NELEMS(pinstr->cmdid) - 1)
				pinstr->cmdid[i++] = c;
		}
		pinstr->cmdid[i] = '\0';

		if (strcmp(pinstr->cmdid, ".") == 0)
			break;

		if (isspace(c)) {
			fscanf(fp, " ");
			c = getc(fp);
		}

		i = 0;
		for (; c != EOF && c != '~'; c = getc(fp)) {
			if (i < NELEMS(pinstr->args) - 1)
				pinstr->args[i++] = c;
		}
		pinstr->args[i--] = '\0';
		while (i >= 0 && isspace(pinstr->args[i]))
			pinstr->args[i--] = '\0';

		s_code_pc++;
	}

	strcpy(s_code[s_code_pc].cmdid, ".");
	strcpy(s_code[s_code_pc].args, "");
	s_code_pc = 0;

	fclose(fp);
}

void register_map_init_fn(map_init_pfn_t fn)
{
	kasserta(s_free_mapinitfn_index < NMAP_INIT_FNS);
	kasserta(fn != NULL);

	s_map_init_table[s_free_mapinitfn_index++] = fn;
}

static void exec_map_init_fns(int mapid)
{
	int i;

	for (i = 0; i < s_free_mapinitfn_index; i++)
		s_map_init_table[i](mapid);
}

/*
 * Frees all sprites and actors and loads the init map file for
 * the map id "data/ixxx.txt".
 */
static void init_map(int id)
{
	free_actors();
	te_free_sprites();
	te_free_shapes();
	free_paths();
	disable_fade();

	s_map_id = id;
	s_win_scheduled = 0;
	s_scheduled_map_exit = -1;
	memset(s_map_exits, 0, sizeof(s_map_exits));

	exec_map_init_fns(id);
	s_got_oxigen = 0;
	load_init_map(id);
	exec_init_map_code();
}

void load_map(int id)
{
	if (kassert_fails(id >= 0)) {
		id = 0;
	}

	te_set_bg_blockset(&main_blockset);
	te_load_map(&main_tileset, get_map_fname(id));
	te_set_tileanim(TI_LAVA, &tileam_lava, 32, 1);
	te_set_tileanim(TI_WATER, &tileam_water, 4, 1);
	te_clear_fg(0, chri(' '));
	init_map(id);
}

/*
 * start_point:
 * 	0 to 3, restars the map and puts player in one of the four positions.
 * 	-1 restarts map from last entering position.
 * 	-2 restarts map from default position.
 */ 
void restart_map(int start_point)
{
	if (kassert_fails(s_map_id >= 0))
		return;

	load_map(s_map_id);
	if (s_actors[AC_COSMONAUT].update == NULL) {
		/* For some reason files didn't load */
		return;
	}
	if (start_point == -1 && s_exit_side_backup >= 0) {
		cosmonaut_enter_map(&s_cosmo_actor_backup,
		       	&s_cosmo_sprite_backup,
			s_exit_side_backup);
	} else {
		s_exit_side_backup = -1;
		cosmonaut_start_at(start_point);
	}
}

void reset_exit_side_backup(void)
{
	s_exit_side_backup = -1;
}

/*
 * 'side' is 0 left, 1 right, 2 up, 3 down.
 */
int map_has_exit(int side)
{
	kasserta(side >= 0 && side <= 3);
	return s_map_exits[side];
}

/* Schedule a teleport to a map id, stargate id. */
void schedule_teleport(int mapid, int gateid, int next_active_gateid)
{
	s_teleport_mapid = mapid;
	s_teleport_gateid = gateid;
	s_teleport_next_active_gateid = next_active_gateid;
}

int is_teleport_scheduled(void)
{
	return s_teleport_mapid >= 0;
}

void teleport(void)
{
	if (!is_teleport_scheduled()) {
		return;
	}

	s_active_stargate_id = s_teleport_next_active_gateid;
	hud_teleported();
	load_map(s_teleport_mapid);
	s_teleport_mapid = -1;
	s_exit_side_backup = -1;
	cosmonaut_start_at_gate(s_teleport_gateid);
}

void schedule_win(void)
{
	s_win_scheduled = 1;
}

int is_win_scheduled(void)
{
	return s_win_scheduled;
}

/*
 * 'side' is 0 left, 1 right, 2 up, 3 down.
 */
void schedule_map_exit(int side)
{
	kasserta(side >= 0 && side <= 3);
	s_scheduled_map_exit = side;
}

int is_map_exit_scheduled(void)
{
	return s_scheduled_map_exit >= 0;
}

void exit_map(void)
{
	int id;

	if (!is_map_exit_scheduled()) {
		return;
	}

	switch (s_scheduled_map_exit) {
	case 0: id = (s_map_id & 0xf0) | ((s_map_id - 1) & 0x0f); break;
	case 1: id = (s_map_id & 0xf0) | ((s_map_id + 1) & 0x0f); break;
	case 2: id = (s_map_id & 0x0f) | ((s_map_id - 16) & 0xf0); break;
	case 3: id = (s_map_id & 0x0f) | ((s_map_id + 16) & 0xf0); break;
	default: id = s_map_id;
	}

	/* backup cosmonaut state */
	s_exit_side_backup = s_scheduled_map_exit;
	s_cosmo_actor_backup = s_actors[AC_COSMONAUT];
	s_cosmo_sprite_backup = *s_cosmo_actor_backup.psp;

	load_map(id);

	/* restore and fix cosmonaut state */
	cosmonaut_enter_map(&s_cosmo_actor_backup, &s_cosmo_sprite_backup,
		s_exit_side_backup);
}

void free_actor(struct actor *pac)
{
	memset(pac, 0, sizeof(*pac));
}

void free_actors(void)
{
	memset(s_actors, 0, sizeof(s_actors));
}

int is_free_actor(struct actor *pac)
{
	return pac->update == NULL;
}

struct actor *get_actor(int id)
{
	kasserta(id >= 0 && id < NACTORS);
	return &s_actors[id];
}

struct actor *get_free_actor(int a, int b)
{
	if (a < 0) {
		a = 0;
	}
	if (b > NACTORS) {
		b = NACTORS;
	}
	while (a < b) {
		if (s_actors[a].update == NULL) {
			return &s_actors[a];
		}
		a++;
	}
	return NULL;
}

/*
 * Register an update function.
 * An update function is used by modules to update themselves per frame.
 */
void register_update_fn(update_pfn_t fn)
{
	kasserta(s_free_updatefn_index < NUPDATE_FNS);
	kasserta(fn != NULL);

	s_update_table[s_free_updatefn_index++] = fn;
}

/*
 * Update funcs are called before update_actors.
 * They are used by modules to update by frame.
 */
void exec_update_fns(void)
{
	int i;

	for (i = 0; i < s_free_updatefn_index; i++)
		s_update_table[i]();
}

/* Call the update function of all actors. */
void update_actors(void)
{
	int i;
	struct actor *ac;

	for (i = 0; i < NACTORS; i++) {
		ac = &s_actors[i];
		if (ac->update != NULL) {
			ac->update(ac);
		}
	}
}

void set_actor_path(struct actor *pac, int path_id, int path_point,
		    int speed, int loop)
{
	init_path_info(&pac->path_info, path_id, path_point, speed, loop);
	if (pac->psp != NULL) {
		pac->psp->x = pac->path_info.x;
		pac->psp->y = pac->path_info.y;
	}
}

/*
 * If change_dir, will change the actor facing direction to follow
 * the path.
 */
void update_actor_path(struct actor *pac, int change_dir)
{
	int n;

	if (!is_path_info_active(&pac->path_info)) {
		return;
	}

	n = FPS_MUL;
	while (n--) {
		update_path_info(&pac->path_info);

		if (kassert_fails(pac->psp != NULL))
			return;

		pac->psp->x = pac->path_info.x;
		pac->psp->y = pac->path_info.y;
		if (change_dir) {
			if (pac->path_info.dir == PATH_RDIR)
				set_actor_dir(pac, RDIR);
			else if (pac->path_info.dir == PATH_LDIR)
				set_actor_dir(pac, LDIR);
		}
	}
}

/* 
 * Sets the actor direction and changes the asigned sprite flip flags.
 * dir is LDIR, RDIR, UDIR, DDIR.
 */
void set_actor_dir(struct actor *pac, int dir)
{
	pac->dir = dir;
	if (pac->psp == NULL || pac->psp->pframe == NULL) 
	{
		return;
	}

	switch (dir) {
	case LDIR: pac->psp->flags |= SP_F_FLIPH; break;
	case DDIR: pac->psp->flags |= SP_F_FLIPV; break;
	case RDIR: pac->psp->flags &= ~SP_F_FLIPH; break;
	case UDIR: pac->psp->flags &= ~SP_F_FLIPV; break;
	default: pac->psp->flags &= ~(SP_F_FLIPH | SP_F_FLIPV); break;
	}
}

/*
 * Fills r with the bounding box of the current frame of the sprite of the
 * actor pa, in world coordinates, displaced to the actor position,
 * and considering the sprite transform flags (FLIPH, FLIPV).
 * Sets r.w and r.h as 0 if there is not a bounding box info; r.x and r.y will
 * be undefined.
 */
void get_actor_bbox(struct rect *r, struct actor *pac)
{
	if (pac->psp == NULL) {
		r->x = r->y = r->w = r->h = 0;
	} else {
		te_get_sprite_bbox(r, pac->psp);
	}
}

int actor_overlaps(struct actor *pa, struct actor *pb)
{
	if (pa->psp == NULL || pb->psp == NULL)
		return 0;
	return te_sprite_bbox_overlaps(pa->psp, pb->psp);
}

#if 0
int actor_overlaps_mask(struct actor *pa, struct actor *pb)
{
	struct rect a, b, r;

	a = pa->psp->pframe->r;
	a.x += te_w2p(pa->psp->x);
	a.y += te_w2p(pa->psp->y);
	b = pb->psp->pframe->r;
	b.x += te_w2p(pb->psp->x);
	b.y += te_w2p(pb->psp->y);
	rect_intersect(&a, &b, &r);
	if (r.w == 0 || r.h == 0) {
		return 0;
	}

	/* rects inside each frame */
	a.x = r.x - a.x;
	a.y = r.y - a.y;
	b.x = r.x - b.x;
	b.y = r.y - b.y;

	pam = pa->psp->pframe->pmask;
	pbm = pb->psp->pframe->pmask;
	for (y = 0; y < r.h; y++) {
		for (x = 0; x < r.w; x++) {
			if ((pam->pixels[(a.y + y) * pam->pitch + a.x + x] &
			     pbm->pixels[(b.y + y) * pbm->pitch + b.x + x])
			    != 0)
			{
				return 1;
			}
		}
	}
}
#endif

void move_actor_x(struct actor *pac)
{
	int i;
	int oldv;

	for (i = 0; i < FPS_MUL; i++) {
		oldv = pac->vx;
		pac->vx += pac->ax;
		if (pac->psp != NULL) {
			pac->psp->x += asr(oldv + pac->vx, 1); 
		}
	}
}

void move_actor_y(struct actor *pac)
{
	int i;
	int oldv;

	for (i = 0; i < FPS_MUL; i++) {
		oldv = pac->vy;
		pac->vy += pac->ay;
		if (pac->psp != NULL) {
			pac->psp->y += asr(oldv + pac->vy, 1); 
		}
	}
}

void move_actor(struct actor *pac)
{
	move_actor_x(pac);
	move_actor_y(pac);
}

int is_blocked_tile_x(int tx, int ty, int left)
{
	int tt;

	tt = te_tile_type(te_bg_xy(tx, ty));
	return (tt & TT_BLOCK) != 0;
}

int is_blocked_tile_y(int tx, int ty, int up)
{
	int tt;

	/*
	if (up)
		return 0;
	*/

	tt = te_tile_type(te_bg_xy(tx, ty));
	return (tt & TT_BLOCK) != 0;
}

int how_far_x(int x0, int y0, int x1, int y1, int delta)
{
	int flip, left, d, x;
	int ty0, ty1, tx, ty;

	left = (delta < 0);
	if (delta < 0) {
		flip = -1;
		delta = -delta;
	} else {
		flip = 1;
	}

	d = 0;
	ty0 = te_w2t(y0);
	ty1 = te_w2t(y1);
	do {
		d = (d + TE_VBTW > delta) ? delta : d + TE_VBTW;
		x = left ? x0 - d : x1 + d;
		tx = te_w2t(x);
		for (ty = ty1; ty >= ty0; ty--) {
			if (!is_blocked_tile_x(tx, ty, left))
				continue;
			delta = x - te_t2w(tx);
			if (left)
				return flip * (d - TE_VBTW + delta -1);
			else
				return flip * (d - delta - 1);
		}
	} while (d < delta);

	return flip * delta;
}

int how_far_y(int x0, int y0, int x1, int y1, int delta)
{
	int flip, up, d, y;
	int tx0, tx1, tx, ty;
	int first_ty;

	up = (delta < 0);
	if (delta < 0) {
		flip = -1;
		delta = -delta;
	} else {
		flip = 1;
	}

	d = 0;
	tx0 = te_w2t(x0);
	tx1 = te_w2t(x1);
	first_ty = up ? te_w2t(y0) : te_w2t(y1);
	do {
		d = (d + TE_VBTW > delta) ? delta : d + TE_VBTW;
		y = up ? y0 - d : y1 + d;
		ty = te_w2t(y);
		for (tx = tx1; tx >= tx0; tx--) {
			if (ty == first_ty || !is_blocked_tile_y(tx, ty, up))
				continue;

			delta = y - te_t2w(ty);
			if (up)
				return flip * (d - TE_VBTW + delta - 1);
			else
				return flip * (d - delta - 1);
		}
	} while (d < delta);

	return flip * delta;
}

int dectime(int t)
{
	return t - FPS_MUL;
}

static void on_frame(void *data)
{
	const struct kernel_device *kd;

	kd = kernel_get_device();

	/*
	 * The first time, set the first state.
	 */
	if (g_state == NULL) {
		switch_to_state(&load_st);
	}

	update_pad();

	if (s_cheats_debug_mode)  {
		if (kd->key_first_pressed(KERNEL_KSC_P))
			s_step_mode = !s_step_mode;
	}

	if (!s_step_mode || kd->key_first_pressed(KERNEL_KSC_SPACE)) {
		update_state();
	}
}

static void on_sound(void *data, unsigned char *samples, int nsamples)
{
	modplay_generate((short *) samples, nsamples, 1);
	mixer_generate((short *) samples, nsamples, 0);
}

static struct kernel_config kcfg = {
	.title = PACKAGE_NAME,
	.canvas_width = TE_SCRW,
	.canvas_height = TE_SCRH,
	.fullscreen = !DEBUG_ON,
	.maximized = 1,
	.frames_per_second = FPS,
	.on_frame = on_frame,
	.on_sound = on_sound,
	.hint_scale_quality = 1,
};

int game_run(void)
{
	int ret;
	const struct kernel_device *d;

	d = kernel_get_device();
	kcfg.canvas_height += pad_scrh();
	ret = d->run(&kcfg, NULL);
	return ret;
}
