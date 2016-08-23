#ifndef FADE_ST_H
#define FADE_ST_H

#ifndef STATE_H
#include "gamelib/state.h"
#endif

extern const struct state fade_st;

void fade_to_state(const struct state *st);

#endif
