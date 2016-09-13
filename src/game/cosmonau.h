/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef COSMONAU_H
#define COSMONAU_H

struct actor;
struct sprite;

struct actor *spawn_cosmonaut(int tx, int ty);

int cosmonaut_how_far_x(struct actor *pac, int d, int *exit);
void cosmonaut_enter_map(struct actor *pac, struct sprite *psp, int exit_side);
void cosmonaut_start_at(int start_point);
void cosmonaut_start_at_gate(int gateid);
void cosmonaut_set_for_demo(int x, int y, int dir, int jumping,
			    int ax, int vx, int ay, int vy);
void cosmonaut_set_dying(int die);
void cosmonaut_set_dying_ns(int die);
int cosmonaut_is_dead(void);
int cosmonaut_is_teleporting(void);
void cosmonaut_set_actor_exit_dirpos(struct actor *parrow);

void cosmonaut_idle(struct actor *pac);
void cosmonaut_down(struct actor *pac);
void cosmonaut_walk(struct actor *pac);
void cosmonaut_jump(struct actor *pac);
void cosmonaut_dielava_start(struct actor *pac);
void cosmonaut_dielava_end(struct actor *pac);
void cosmonaut_die(struct actor *pac);

void cosmonau_init(void);

#endif
