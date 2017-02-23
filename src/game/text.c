/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "text.h"
#include "readlin.h"
#include "gamelib/lang.h"
#include "gamelib/vfs.h"
#include "kernel/kernel.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include <ctype.h>
#include <stdlib.h>

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#include <string.h>

static const struct lang_equiv {
	const char *src;
	const char *dst;
} s_lang_equivs[] = {
	{ "ca", "es" },
	{ "eu", "es" },
	{ "gl", "es" },
	{ "an", "es" },
	{ "la", "il" },
};

enum {
	NLANGS = 32
};

struct lang_info {
	char *name;
	char *code;
};

static struct lang_info s_langlist[NLANGS];
static int s_nlangs;
static char s_curlang[5] = "en";

/*
 * Maximum number of entries allowed.
 * Must not be more than 64K.
 */
#define NSYMS		256

/* Closest prime to NSYMS / 4. */
#define SYMTABSZ	61

struct sym {
	char *key;
	char *val;
	unsigned short next;		/* index into symlist; 0 is no next */
};

/*
 * Nodes for the s_symtab hash table.
 * The symbol at index 0 is never used.
 */
static struct sym s_symlist[NSYMS];

/*
 * Hash table of indexes into s_symlist.
 * 0 means that the bucket is empty.
 */
static unsigned short s_symtab[SYMTABSZ];

/* Next free symbol in s_symlist. Note: 0 not used. */
static int s_nsyms = 1;

static int hash(const char *p, unsigned int tabsz)
{
	unsigned int h;

	h = 0;
	while (*p != '\0') {
		h = 31 * h + (unsigned char) *p;
		p++;
	}

	return h % tabsz;
}

/*
 * Lookups the string key in s_symtab.
 * If !insert, returns the entry or NULL if it is not in the table.
 * If insert, inserts the key, val in the table, overwriting if there
 * was already the same key. In this case key and val are duplicated
 * and allocated, you can free the original key and val passed if needed.
 * If there is no memory, nothing is inserted.
 */
struct sym *lookup(const char *key, int insert, const char *val)
{
	int h, k;
	struct sym *nod;
	char *dupk, *dupv;

	if (key == NULL || (insert && val == NULL))
		return NULL;

	h = hash(key, SYMTABSZ);
	for (k = s_symtab[h]; k != 0; k = s_symlist[k].next) {
		if (strcmp(key, s_symlist[k].key) == 0) {
			if (insert) {
				dupv = dupstr(val);
				if (dupv != NULL) {
					free(s_symlist[k].val);
					s_symlist[k].val = dupv;
				}
			}
			return &s_symlist[k];
		}
	}

	if (insert && s_nsyms < NSYMS) {
		dupk = dupstr(key);
		dupv = dupstr(val);
		if (dupk == NULL || dupv == NULL) {
			if (dupk != NULL)
				free(dupk);
			if (dupv != NULL)
				free(dupv);
		} else {
			nod = &s_symlist[s_nsyms];
			nod->next = s_symtab[h];
			s_symtab[h] = (unsigned short) s_nsyms;
			s_nsyms++;
			nod->key = dupk;
			nod->val = dupv;
			return nod;
		}
	}

	return NULL;
}

static void free_symtab(void)
{
	int i;

	for (i = 1; i < s_nsyms; i++) {
		free(s_symlist[i].key);
		s_symlist[i].key = NULL;
		free(s_symlist[i].val);
		s_symlist[i].val = NULL;
	}

	s_nsyms = 1;

	for (i = 0; i < SYMTABSZ; i++)
		s_symtab[i] = 0;
}

static int get_lang_code_index(const char *code)
{
	int i;

	for (i = 0; i < s_nlangs; i++) {
		if (strcmp(code, s_langlist[i].code) == 0) {
			return i;
		}
	}

	return -1;
}

int get_cur_lang_index(void)
{
	return get_lang_code_index(s_curlang);
}

/* Loads data/lang[code].txt. Format is:
 *
 *	ENGLISH_LINE
 *	TRANSLATED_LINE
 *	...
 *	~
 *
 *	For example:
 *
 *	HELLO
 *	HOLA
 *	BYE
 *	ADIOS
 *	~
 */
void load_lang(const char *lang_code)
{
	FILE *fp;
	static char fname[32];
	char key[READLIN_LINESZ];
	char val[READLIN_LINESZ];
	int i, keylen, vallen;

	free_symtab();

	snprintf(s_curlang, sizeof(s_curlang), "%s", lang_code);

	snprintf(fname, sizeof(fname), "data/lang%s.txt", lang_code);
	if ((fp = open_file(fname, NULL)) == NULL) {
		if (strcmp(lang_code, "en") != 0) {
			ktrace("language file does not exist: %s", fname);
		}
		return;
	}

	for (i = 1; i < NSYMS; i++) {
		keylen = readlin(fp, key);
		vallen = readlin(fp, val);
		if (keylen == -1 || vallen == -1)
			break;
		lookup(key, 1, val);
	}

	fclose(fp);
}

const char *translate(const char *key)
{
	struct sym *psym;

	psym = lookup(key, 0, NULL);
	if (psym == NULL)
		return key;
	return psym->val;
}

static void add_lang(const char *name, const char *code)
{
	char *dname, *dcode;

	if (s_nlangs >= NLANGS)
		return;

	dname = dupstr(name);
	dcode = dupstr(code);
	if (dname == NULL || dcode == NULL) {
		if (dname != NULL)
			free(dname);
		if (dcode != NULL)
			free(dcode);
	} else {
		s_langlist[s_nlangs].name = dname;
		s_langlist[s_nlangs].code = dcode;
		s_nlangs++;
	}
}

static void free_lang_list(void)
{
	int i;

	for (i = 0; i < s_nlangs; i++) {
		free(s_langlist[i].name);
		s_langlist[i].name = NULL;
		free(s_langlist[i].code);
		s_langlist[i].code = NULL;
	}
	s_nlangs = 0;
}

static void set_lang_equivalences(void)
{
	int i;

	reset_lang_equivalences();
	for (i = 0; i < NELEMS(s_lang_equivs); i++) {
		if (!is_lang_code_listed(s_lang_equivs[i].src)) {
			add_lang_equivalence(s_lang_equivs[i].src,
					     s_lang_equivs[i].dst);
		}
	}
}

/* Loads data/langlist.txt . Format is:
 *
 * 	LANGUAGE_NAME
 * 	LANGUAGE_CODE
 * 	...
 * 	~
 *
 * 	For example:
 *
 * 	ESPANOL
 * 	es
 * 	ITALIANO
 * 	it
 * 	~
 *
 * English must not be added since it is precompiled by default.
 */
void load_lang_list(void)
{
	FILE *fp;
	char key[READLIN_LINESZ];
	char val[READLIN_LINESZ];
	int i, rkey, rval;

	free_lang_list();
	add_lang("ENGLISH", "en");

	if ((fp = open_file("data/langlist.txt", NULL)) == NULL)
		return;

	for (i = 0; i < NLANGS; i++) {
		rkey = readlin(fp, key);
		rval = readlin(fp, val);
		if (rkey == -1 || rval == -1)
			break;
		add_lang(key, val);
	}

	fclose(fp);
	set_lang_equivalences();
}

/* Get the number of supported languages.
 * English + languages listed in langlist.txt .
 */
int get_nlangs(void)
{
	return s_nlangs;
}

const char *get_lang_name(int i)
{
	if (i < 0 || i >= s_nlangs)
		return "";

	return s_langlist[i].name;
}

const char *get_lang_code(int i)
{
	if (i < 0 || i >= s_nlangs)
		return "";

	return s_langlist[i].code;
}

/* Returns true if a language code is listed in the language list.
 * "en" is always available.
 */
int is_lang_code_listed(const char *code)
{
	return get_lang_code_index(code) >= 0;
}

void text_init(void)
{
	s_nlangs = 0;
	kasserta(sizeof(s_curlang) >= 3);
	strcpy(s_curlang, "en");
	add_lang("ENGLISH", "en");
}

/* TODO: text_done */
