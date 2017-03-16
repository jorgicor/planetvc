/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "prefs.h"
#include "readlin.h"
#include "confpath.h"
#include "gamelib/vfs.h"
#include "cbase/kassert.h"
#include "cbase/cbase.h"
#include <string.h>
#include <stdlib.h>

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

enum {
	NPREFS = 32,
	NCHARS = 31,
};

struct pref {
	char key[NCHARS + 1];
	char val[NCHARS + 1];
};

struct pref s_prefs[NPREFS];

/* internal means that is a preference that we know we use in the current
 * version of the game. That is, we install it through code.
 * !internal us used when we load from the preferences file.
 * So, if we load a preference, but it was not alerady in the table
 * it is not added.
 */
static void insert(const char *key, const char *val, int internal)
{
	int i;
	int success;

	if (key == NULL || val == NULL)
		return;

	success = 0;
	for (i = 0; i < NPREFS; i++) {
		if (s_prefs[i].key[0] == '\0') {
			success = 1;
			if (internal) {
				snprintf(s_prefs[i].key, NCHARS + 1, "%s",
					 key);
				snprintf(s_prefs[i].val, NCHARS + 1, "%s",
					 val);
			}
			break;
		} else if (strcmp(s_prefs[i].key, key) == 0) {
			success = 1;
			snprintf(s_prefs[i].val, NCHARS + 1, "%s", val);
			break;
		}
	}

	kassert(success);
}

/* Never returns NULL.
 * Returns "*" if the preference was not defined.
 */
const char *get_preference(const char *key)
{
	int i;

	for (i = 0; i < NPREFS; i++) {
		if (s_prefs[i].key[0] == '\0') {
			break;
		} else if (strcmp(s_prefs[i].key, key) == 0) {
			return s_prefs[i].val;
		}
	}

	return "*";
}

void set_preference(const char *key, const char *val)
{
	insert(key, val, 1);
}

void set_preference_int(const char *key, int val)
{
	char valstr[NCHARS + 1];

	snprintf(valstr, sizeof(valstr), "%d", val);
	set_preference(key, valstr);
}

/* Returns NULL if failure, or a pointer to path, that will be filled.
 * path must have space for OPEN_FILE_MAX_PATH_LEN + 1 characters.
 */
static char *mkpath(char path[])
{
	static int once = 0;
	static const char *fname = "prefs.txt";
	const char *cpath;

	cpath = confpath_get();
	if (!once) {
		if (cpath == NULL)
			ktrace("config path is NULL");
		else
			ktrace("config path is %s", cpath);
		once = 1;
	}

	if (cpath == NULL)
		return NULL;

	if (kassert_fails(strlen(cpath) + strlen(fname) <=
			  OPEN_FILE_MAX_PATH_LEN))
	{
		return NULL;
	}

	strcpy(path, cpath);
	strcat(path, fname);
	return path;
}

void load_prefs(void)
{
	FILE *fp;
	int i, rkey, rval;
	char key[READLIN_LINESZ];
	char val[READLIN_LINESZ];
	char path[OPEN_FILE_MAX_PATH_LEN + 1];

	if (mkpath(path) == NULL)
		return;

	fp = fopen(path, "r");
	if (fp == NULL)
		return;

	for (i = 0; i < NELEMS(s_prefs); i++) {
		rkey = readlin(fp, key);
		rval = readlin(fp, val);
		if (rkey == -1 || rval == -1)
			break;
		insert(key, val, 0);
	}

	fclose(fp);
}

void save_prefs(void)
{
	int i;
	FILE *fp;
	char path[OPEN_FILE_MAX_PATH_LEN + 1];

	if (mkpath(path) == NULL)
		return;

	if ((fp = fopen(path, "w")) == NULL)
		return;

	set_preference("default", "0");
	for (i = 0; i < NPREFS; i++) {
		if (s_prefs[i].key[0] == '\0')
			break;
		fputs(s_prefs[i].key, fp);
		fputc('\n', fp); 
		fputs(s_prefs[i].val, fp);
		fputc('\n', fp); 
	}
	fputs("~\n", fp);

	fclose(fp);
}

int is_music_enabled(void)
{
	return strcmp(get_preference("music"), "1") == 0;
}

void prefs_init(void)
{
	memset(s_prefs, 0, sizeof(s_prefs));
	set_preference("default", "1");
	set_preference("lang", "en");
	set_preference("music", "1");
	set_preference("volume", "100");
	set_preference("stargate", "0");
	set_preference("connectq", "0");
	set_preference("autoconnect", "0");
}
