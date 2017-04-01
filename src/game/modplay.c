/*
Copyright (c) 2016-2017 Jorge Giner Cordero

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "modplay.h"
#include "gamelib/vfs.h"
#include "cbase/kassert.h"
#include "libxmp-lite/xmp.h"

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h>
#endif

#include <string.h>
#include <limits.h>

enum {
	ST_EMPTY,
	ST_STOP,
	ST_PLAY
};

static int s_state;
static xmp_context s_xmpc;
static int s_loop;
static char s_modname[13] = { '\0' };

/* 
 * Loads a MOD, XM file.
 * modname must be less than 12 characters, dot and extension included.
 */
void modplay_load(const char *modname)
{
	FILE *fp;
	unsigned int fsize;
	long cfsize;
	char path[] = "data/12345678.ext";
	unsigned char *fdata;

	if (s_state != ST_EMPTY)
		modplay_free();

	if (modname == NULL)
		return;

	kassert(strlen(modname) <= 8 + 1 + 3);

	snprintf(path, sizeof(path), "data/%s", modname);
	if ((fp = open_file(path, &fsize)) == NULL)
		return;

	if (fsize == UINT_MAX) {
		fseek(fp, 0, SEEK_END);
		cfsize = ftell(fp);
		if (cfsize == EOF || (unsigned long) cfsize > UINT_MAX - 1)
			goto fail; 
		rewind(fp);
		fsize = (unsigned int) cfsize;
	}

	fdata = malloc(fsize);
	if (fdata == NULL)
		goto fail;

	fread(fdata, 1, fsize, fp);
	fclose(fp);

	s_xmpc = xmp_create_context();
	if (xmp_load_module_from_memory(s_xmpc, fdata, fsize) != 0) {
		xmp_free_context(s_xmpc);
	} else {
		snprintf(s_modname, sizeof(s_modname), "%s", modname);
		s_state = ST_STOP;
	}

	free(fdata);
	return;
	
fail:	fclose(fp);
}

/* Returns the loaded mod name or empty string. */
const char *modplay_get_modname(void)
{
	return s_modname;
}

int modplay_is_playing(void)
{
	return s_state == ST_PLAY;
}

void modplay_play(int loop)
{
	if (s_state == ST_PLAY)
		modplay_stop();

	if (s_state == ST_STOP) {
		if (kassert(xmp_start_player(s_xmpc, 44100, 0) == 0)) {

			/* This line fixes a SEGFAULT on Windows.
			 * By passing NULL we reset the player context.
			 *
			 * How to repro:
			 * 	Start with music on.
			 * 	Set music off on menu.
			 * 	Set music on again.
			 * 	We got SEGFAULT at libxmp-lite/player.c
			 * 	xmp_play_buffer():1511
			 * 	from modplay_generate()
			 */
			xmp_play_buffer(s_xmpc, NULL, 0, 0);

			s_loop = loop;
			s_state = ST_PLAY;
		}
	}
}

void modplay_stop(void)
{
	if (s_state == ST_PLAY) {
		xmp_stop_module(s_xmpc);
		s_state = ST_STOP;
	}
}

void modplay_generate(short *ptr, int nsamples, int fill_silence)
{
	if (s_state != ST_PLAY) {
		if (fill_silence)
			memset(ptr, 0, nsamples * 4);
		return;
	}
	xmp_play_buffer(s_xmpc, ptr, nsamples * 4, !s_loop);
}

void modplay_free(void)
{
	if (s_state != ST_EMPTY) {
		s_modname[0] = '\0';
		xmp_free_context(s_xmpc);
		s_state = ST_EMPTY;
	}
}

void modplay_done(void)
{
	modplay_free();
}
