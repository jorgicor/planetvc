#ifndef MODPLAY_H
#define MODPLAY_H

void modplay_load(const char *modname);
const char *modplay_get_modname(void);
int modplay_is_playing(void);
void modplay_play(int loop);
void modplay_stop(void);
void modplay_generate(short *ptr, int nsamples, int fill_silence);
void modplay_free(void);
void modplay_done(void);

#endif
