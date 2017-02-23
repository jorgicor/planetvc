/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef TEXT_H
#define TEXT_H

void load_lang_list(void);
int is_lang_code_listed(const char *code);
int get_nlangs(void);
const char *get_lang_name(int i);
const char *get_lang_code(int i);
int get_cur_lang_index(void);

void load_lang(const char *lang_code);
const char *translate(const char *key);

#define _(str)	translate(str)

void text_init(void);
void text_done(void);

#endif
