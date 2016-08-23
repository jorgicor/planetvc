#ifndef PREFS_H
#define PREFS_H

const char *get_preference(const char *key);
void set_preference(const char *key, const char *val);
void set_preference_int(const char *key, int val);

void load_prefs(void);
void save_prefs(void);

int is_music_enabled(void);

void prefs_init(void);

#endif
