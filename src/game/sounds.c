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

#include "sounds.h"
#include "cbase/cbase.h"
#include "gamelib/vfs.h"
#include "gamelib/wav.h"
#include "cbase/kassert.h"

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

#include <string.h>

enum {
	NSOUNDS = 64
};

static int s_load_index;
static int s_free_index;

/* pdup is NULL or points to a previous entry with the same 'name'. */
struct wav_file {
	struct wav_file *pdup;
	struct wav **pp_wav;
	const char *name;		/* Only 8 characters allowed + '\0'. */
};

static struct wav_file s_sounds[NSOUNDS];

static int load_sound_num(int i)
{
	FILE *fp;
	struct wav *p_wav;
	struct wav_file *wavf;
	char path[24];

	if (i >= s_free_index)
		return E_SOUNDS_LOAD_EOF;

	wavf = &s_sounds[i];
	if (wavf->pdup == NULL) {
		snprintf(path, sizeof(path), "data/%s.wav", wavf->name);
		fp = open_file(path, NULL);
		if (fp == NULL)
			return E_SOUNDS_LOAD_ERROR;

		p_wav = load_wav_fp(fp);
		fclose(fp);
	} else {
		p_wav = *wavf->pdup->pp_wav;
	}

	if (p_wav == NULL)
		return E_SOUNDS_LOAD_ERROR;

	*wavf->pp_wav = p_wav;
	s_load_index++;
	return E_SOUNDS_LOAD_OK;
}

const char *cur_sound_name(void)
{
	if (s_load_index < 0 || s_load_index >= s_free_index)
		return "* none *";

	return s_sounds[s_load_index].name;
}

int load_next_sound(void)
{
	return load_sound_num(s_load_index);
}

/* Note that *fname is not copied, it must survive.
 * Also allows for registering the same name: the wav will only be loaded
 * once but the pointers of wveryone registered will be set.
 */
void register_sound(struct wav **ppwav, const char *fname)
{
	int i;
	struct wav_file *pwavf;
	struct wav_file *pdupf;

	kasserta(s_free_index < NSOUNDS);
	kasserta(fname != NULL);
       	kasserta(strlen(fname) <= 8);

	pdupf = NULL;
	for (i = 0; i < s_free_index; i++) {
		if (strcmp(s_sounds[i].name, fname) == 0) {
			pdupf = &s_sounds[i];
			break;
		}
	}

	pwavf = &s_sounds[s_free_index++];
	pwavf->pdup = pdupf;
	pwavf->pp_wav = ppwav;
	pwavf->name = fname;
}

void sounds_done(void)
{
	int i;
	struct wav_file *wavf;

	for (i = 0; i < s_free_index; i++) {
		wavf = &s_sounds[i];
		if (*wavf->pp_wav != NULL) {
			if (wavf->pdup == NULL) {
				free_wav(*wavf->pp_wav);
			}
			*wavf->pp_wav = NULL;
		}
	}

	s_free_index = 0;
	s_load_index = 0;
}
