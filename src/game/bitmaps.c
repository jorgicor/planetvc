/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "bitmaps.h"
#include "cbase/cbase.h"
#include "gamelib/vfs.h"
#include "gamelib/bmp.h"
#include "cbase/kassert.h"

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>

enum {
	NBITMAPS = 64
};

static int s_load_index;
static int s_free_index;

struct bmp_file {
	struct bmp_file *pdup;
	struct bmp **pp_bmp;
	const char *name;		/* Only 8 characters allowed + '\0'. */
	int use_key_color;
	unsigned int key_color;
};

static struct bmp_file s_bitmaps[NBITMAPS];

static int load_bitmap_num(int i)
{
	FILE *fp;
	struct bmp *p_bmp;
	struct bmp_file *bmpf;
	char path[24];

	if (i >= s_free_index)
		return E_BITMAPS_LOAD_EOF;

	bmpf = &s_bitmaps[i];
	if (bmpf->pdup == NULL) {
		snprintf(path, sizeof(path), "data/%s.bmp", bmpf->name);
		fp = open_file(path, NULL);
		if (fp == NULL)
			return E_BITMAPS_LOAD_ERROR;

		p_bmp = load_bmp_fp(fp, NULL);
		fclose(fp);
	} else {
		p_bmp = *bmpf->pdup->pp_bmp;
	}

	if (p_bmp == NULL)
		return E_BITMAPS_LOAD_ERROR;

	*bmpf->pp_bmp = p_bmp;
	p_bmp->use_key_color = bmpf->use_key_color != 0;
	p_bmp->key_color = bmpf->key_color;

	s_load_index++;
	return E_BITMAPS_LOAD_OK;
}

const char *cur_bitmap_name(void)
{
	if (s_load_index < 0 || s_load_index >= s_free_index)
		return "* none *";

	return s_bitmaps[s_load_index].name;
}

int load_next_bitmap(void)
{
	return load_bitmap_num(s_load_index);
}

/* If a bitmap is already registered with the same fname, use_keycolor and
 * keycolor (if use_keycolor), then we will point to the previous bitmap, so
 * the bitmap is not loaded again.
 */
void register_bitmap(struct bmp **ppbmp, const char *fname, int use_keycolor,
	       unsigned int keycolor)
{
	int i;
	struct bmp_file *pbmpf;
	struct bmp_file *pdupf;

	kasserta(s_free_index < NBITMAPS);
	kasserta(fname != NULL);
       	kasserta(strlen(fname) <= 8);

	pdupf = NULL;
	for (i = 0; i < s_free_index; i++) {
		if (strcmp(s_bitmaps[i].name, fname) == 0 &&
			s_bitmaps[i].use_key_color == use_keycolor &&
			(!use_keycolor || s_bitmaps[i].key_color == keycolor))
	       	{
			pdupf = &s_bitmaps[i];
			break;
		}
	}

	pbmpf = &s_bitmaps[s_free_index++];
	pbmpf->pdup = pdupf;
	pbmpf->pp_bmp = ppbmp;
	pbmpf->name = fname;
	pbmpf->use_key_color = use_keycolor;
	pbmpf->key_color = keycolor;
}

void bitmaps_done(void)
{
	int i;
	struct bmp_file *bmpf;

	for (i = 0; i < s_free_index; i++) {
		bmpf = &s_bitmaps[i];
		if (*bmpf->pp_bmp != NULL) {
			if (bmpf->pdup == NULL) {
				free_bmp(*bmpf->pp_bmp, 1);
			}
			*bmpf->pp_bmp = NULL;
		}
	}

	s_free_index = 0;
	s_load_index = 0;
}
