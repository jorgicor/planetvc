/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef GAME_H
#define GAME_H

#ifndef STATE_H
#include "gamelib/state.h"
#endif

#ifndef PATH_H
#include "path.h"
#endif

/*
 * game.c
 */

/* Directions. */
enum {
	RDIR,
	LDIR,
	UDIR,
	DDIR
};

enum {
	PATH_STATE_INACTIVE,
	PATH_STATE_ACTIVE,
	PATH_STATE_REACHED
};

struct sprite;
struct frame;

/* An actor is valid is update is != NULL.
 * psp can be NULL.
 */
struct actor {
	void (*update)(struct actor *);
	struct sprite *psp;
	struct path_info path_info;
	int vx, vy;
	int ax, ay;
	int t;
	int i0, i1, i2;
	struct actor *pac2;
	unsigned char dir;
	unsigned char type;
};

int game_run(void);

typedef struct actor *(*spawn_pfn_t)(int x, int y);
void register_spawn_fn(const char *id, spawn_pfn_t fn);

typedef void (*map_init_pfn_t)(int mapid);
void register_map_init_fn(map_init_pfn_t fn);

typedef void (*update_pfn_t)(void);
void register_update_fn(update_pfn_t fn);
void exec_update_fns(void);

void load_map(int id);
void restart_map(int start_point);
int map_has_exit(int side);
void schedule_map_exit(int side);
int is_map_exit_scheduled(void);
void exit_map(void);
void schedule_teleport(int mapid, int gateid, int next_active_gateid);
int is_teleport_scheduled(void);
void teleport(void);
void reset_exit_side_backup(void);
void schedule_win(void);
int is_win_scheduled(void);

struct actor *get_actor(int id);
struct actor *get_free_actor(int a, int b);
void update_actors(void);
void free_actor(struct actor *pac);
void free_actors(void);
void set_actor_dir(struct actor *pac, int dir);
void set_actor_path(struct actor *pac, int path_id, int path_point,
		    int speed, int loop);
void update_actor_path(struct actor *pac, int change_dir);

struct rect;

int is_blocked_tile_x(int tx, int ty, int up);
int is_blocked_tile_y(int tx, int ty, int up);
int how_far_x(int x0, int y0, int x1, int y1, int delta);
int how_far_y(int x0, int y0, int x1, int y1, int delta);
int bbox_overlaps(struct rect *a, struct rect *b);
void get_actor_bbox(struct rect *r, struct actor *pa);
int actor_overlaps(struct actor *pa, struct actor *pb);
void move_actor(struct actor *pac);
void move_actor_x(struct actor *pac);
void move_actor_y(struct actor *pac);
int is_free_actor(struct actor *pac);

/*
 * Sprite slots.
 */

enum {
	SP_STARGATE_0,
	SP_STARGATE_1,
	SP_STARGATE_LAST = SP_STARGATE_1,
	SP_PLATFORM_0,
	SP_PLATFORM_1,
	SP_PLATFORM_2,
	SP_PLATFORM_3,
	SP_PLATFORM_LAST = SP_PLATFORM_3,
	SP_COSMONAUT,
	SP_OXIGEN_0,
	SP_OXIGEN_1,
	SP_OXIGEN_2,
	SP_OXIGEN_LAST = SP_OXIGEN_2,
	SP_ENEMY_0,
	SP_ENEMY_1,
	SP_ENEMY_2,
	SP_ENEMY_3,
	SP_ENEMY_4,
	SP_ENEMY_5,
	SP_ENEMY_LAST = SP_ENEMY_5,
	SP_ESHOT_0,
	SP_ESHOT_1,
	SP_ESHOT_2,
	SP_ESHOT_3,
	SP_ESHOT_4,
	SP_ESHOT_5,
	SP_ESHOT_LAST = SP_ESHOT_5,
	SP_BALLOON,
	SP_BALLOON_ICON,
	SP_ARROW,
	SP_HUD_COSMO,
	SP_HUD_X,
	SP_HUD_DIG0,
	SP_HUD_DIG1,
};

/*
 * Actor slots.
 */

enum {
	AC_DEMOSAVE,
	AC_STARGATE_0,
	AC_STARGATE_1,
	AC_STARGATE_LAST = AC_STARGATE_1,
	AC_PLATFORM_0,
	AC_PLATFORM_1,
	AC_PLATFORM_2,
	AC_PLATFORM_3,
	AC_PLATFORM_LAST = AC_PLATFORM_3,
	AC_COSMONAUT,
	AC_OXIGEN_0,
	AC_OXIGEN_1,
	AC_OXIGEN_2,
	AC_OXIGEN_LAST = AC_OXIGEN_2,
	AC_ENEMY_0,
	AC_ENEMY_1,
	AC_ENEMY_2,
	AC_ENEMY_3,
	AC_ENEMY_4,
	AC_ENEMY_5,
	AC_ENEMY_LAST = AC_ENEMY_5,
	AC_ESHOT_0,
	AC_ESHOT_1,
	AC_ESHOT_2,
	AC_ESHOT_3,
	AC_ESHOT_4,
	AC_ESHOT_5,
	AC_ESHOT_LAST = AC_ESHOT_5,
	AC_BALLOON,
	AC_ARROW,
	AC_HUD,
	AC_HUD_RESTART,
	AC_MENU,
	AC_CHEATS,
	AC_GAMEOVER,
	AC_HISCORE,
	NACTORS
};

/*
 * Game can go at 30 or 60 FPS.
 *
 * We specify animation speeds as if we were at 30 fps and we call
 * te_set_half_speed_mode(1) if we go actually at 60 fps. 
 *
 * Movement (speed, acceleration, etc) is expressed as if we were at 60 fps.
 * Movement code is ran twice if we go actually at 30 fps.
 */

enum {
	FPS_FULL = 60,
	FPS_HALF = FPS_FULL / 2,
	AM_SEC = FPS_HALF,
	FPS = FPS_HALF,
	FPS_MUL = FPS_FULL / FPS,
	IS_FULL_FPS = (FPS == FPS_FULL),
};

enum {
	DIFFICULTY_EXPERT,
	DIFFICULTY_BEGINNER,
	NDIFFICULTY_LEVELS
};

int get_difficulty(void);
void set_difficulty(int difficulty);

int tokscanf(char *str, const char *fmt, ...);
void scroll_to_map(int mapid);
void exec_init_map_code(void);
int dectime(int t);

void game_init(void);

#endif
