/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef SOUNDS_H
#define SOUNDS_H

enum {
	E_SOUNDS_LOAD_OK,
	E_SOUNDS_LOAD_EOF,
	E_SOUNDS_LOAD_ERROR,
	E_SOUNDS_BIGFNAME,
};

struct wav;

int load_next_sound(void);
const char *cur_sound_name(void);

void register_sound(struct wav **ppwav, const char *fname);

void sounds_done(void);

#endif
