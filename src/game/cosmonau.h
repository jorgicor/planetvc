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
