#ifndef OXIGEN_H
#define OXIGEN_H

struct frame;

extern int s_got_oxigen;
extern struct frame fr_oxigen_shine_0;

void oxigen_init(void);
struct actor *spawn_oxigen(int tx, int ty);

#endif
