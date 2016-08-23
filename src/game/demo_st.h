#ifndef DEMO_ST_H
#define DEMO_ST_H

#ifndef STATE_H
#include "gamelib/state.h"
#endif

extern const struct state demo_st;

int demo_is_key_down(int key);

#endif
