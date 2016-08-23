#ifndef STARGATE_H
#define STARGATE_H

void stargate_init(void);

struct actor;

extern int s_active_stargate_id;

int stargate_check_teleport(struct actor *pac, struct actor *pcosmo, int oldx);
int stargate_get_goto_mapid(struct actor *pac);
int stargate_get_goto_gateid(struct actor *pac);
int stargate_get_next_active_gateid(struct actor *pac);
int stargate_get_id(struct actor *pac);
int stargate_get_arrow_dirpos(struct actor *pac);

void stargate_get_center(struct actor *pac, int *cx, int *cy);
void activate_stargate(struct actor *pac);
void deactivate_stargate(struct actor *pac);

#endif
