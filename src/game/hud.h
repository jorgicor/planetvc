/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef HUD_H
#define HUD_H

int is_restart_scheduled(void);
int is_gameover_scheduled(void);
int is_training(void);
int get_nvisited_maps(void);

void hud_teleported(void);
void hud_game_started(void);

void hud_init(void);

#endif
