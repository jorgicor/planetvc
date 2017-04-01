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

#include "cheats.h"
#include "tilengin.h"
#include "strdraw.h"
#include "game.h"
#include "data.h"
#include "initfile.h"
#include "prefs.h"
#include "kernel/kernel.h"
#include "cfg/cfg.h"

static const int s_keys[] = {
	KERNEL_KSC_A,
	KERNEL_KSC_B,
	KERNEL_KSC_U,
	KERNEL_KSC_S,
	KERNEL_KSC_I,
	KERNEL_KSC_M,
	KERNEL_KSC_B,
	KERNEL_KSC_E,
	KERNEL_KSC_L,
};

int s_cheats_debug_mode;
int s_cheats_demosave_mode;
int s_cheats_translator_mode;

static int s_key_i;
static int s_wait_code;

static const int s_keys_sg[] = {
	KERNEL_KSC_S,
	KERNEL_KSC_T,
	KERNEL_KSC_A,
	KERNEL_KSC_R,
	KERNEL_KSC_G,
	KERNEL_KSC_A,
	KERNEL_KSC_T,
	KERNEL_KSC_E,
};

static int s_key_sg_i;

static int any_key_first_pressed(void)
{
	const struct kernel_device *kd;
	int i;

	kd = kernel_get_device();
	for (i = 0; i < KERNEL_KSC_ESC + 1; i++) {
		if (kd->key_first_pressed(i))
			return 1;
	}

	return 0;
}

static void draw_cheats(void)
{
	if (s_cheats_debug_mode)
		te_set_fg_xy(TE_FMW - 3, 0, 0, chri('1'));
	if (s_cheats_demosave_mode)
		te_set_fg_xy(TE_FMW - 2, 0, 0, chri('2'));
	if (s_cheats_translator_mode)
		te_set_fg_xy(TE_FMW - 1, 0, 0, chri('3'));
}

static void check_cheats(void)
{
	const struct kernel_device *kd;
	int reset;

	draw_cheats();

	kd = kernel_get_device();
	reset = 0;

	/* Always reset if we press the first character, that must not
	 * repeat in the word.
	 */
	if (kd->key_first_pressed(s_keys[0])) {
		s_key_i = 1;
		s_wait_code = 0;
		return;
	}

	if (s_wait_code) {
		reset = 0;
		if (kd->key_first_pressed(KERNEL_KSC_1)) {
			reset = 1;
			s_cheats_debug_mode = 1;
		} else if (kd->key_first_pressed(KERNEL_KSC_2)) {
			reset = 1;
			if (!initfile_getvar("demo_version")) {
				s_cheats_debug_mode = 1;
				s_cheats_translator_mode = 1;
			}
		} else if (kd->key_first_pressed(KERNEL_KSC_3)) {
			reset = 1;
			if (PP_DEBUG && !initfile_getvar("demo_version")) {
				s_cheats_demosave_mode = 1;
			}
		} else if (any_key_first_pressed()) {
			reset = 1;
		}
	} if (kd->key_first_pressed(s_keys[s_key_i])) {
		s_key_i++;
		if (s_key_i == NELEMS(s_keys)) {
			s_wait_code = 1;
		}
	} else if (s_key_i > 0 && any_key_first_pressed()) {
		reset = 1;
	}

	if (reset) {
		s_key_i = 0;
		s_wait_code = 0;
	}
}

static void check_stargate(void)
{
	const struct kernel_device *kd;
	int reset;

	kd = kernel_get_device();
	reset = 0;

	/* Always reset if we press the first character, that must not
	 * repeat in the word.
	 */
	if (kd->key_first_pressed(s_keys_sg[0])) {
		s_key_sg_i = 1;
		return;
	}

	if (kd->key_first_pressed(s_keys_sg[s_key_sg_i])) {
		s_key_sg_i++;
		if (s_key_sg_i == NELEMS(s_keys_sg)) {
			reset = 1;
			set_preference_int("stargate", 1);
			save_prefs();
			apply_stargate_symbols();
		}
	} else if (s_key_sg_i > 0 && any_key_first_pressed()) {
		reset = 1;
	}

	if (reset) {
		s_key_sg_i = 0;
	}
}

static void cheats_update(struct actor *pac)
{
	check_cheats();
	check_stargate();
}

void spawn_cheats(void)
{
	struct actor *pac;

	pac = get_actor(AC_CHEATS);
	if (!is_free_actor(pac))
		return;

	s_wait_code = 0;
	s_key_i = 0;
	s_key_sg_i = 0;
	pac->update = cheats_update;
}

void cheats_init(void)
{
	s_cheats_debug_mode = 0;
	s_cheats_demosave_mode = 0;
	s_cheats_translator_mode = 0;
}
