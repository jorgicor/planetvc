#ifndef MENU_ST_H
#define MENU_ST_H

#ifndef STATE_H
#include "gamelib/state.h"
#endif

extern const struct state menu_st;

void load_defined_keys(void);
void save_defined_keys(void);

void menu_st_init(void);

#endif
