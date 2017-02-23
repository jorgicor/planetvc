/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "cheats.h"
#include "tilengin.h"
#include "strdraw.h"
#include "game.h"
#include "initfile.h"
#include "kernel/kernel.h"

static int s_keys[] = {
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

static void cheats_update(struct actor *pac)
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
			if (!initfile_getvar("demo_version")) {
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

void spawn_cheats(void)
{
	struct actor *pac;

	pac = get_actor(AC_CHEATS);
	if (!is_free_actor(pac))
		return;

	s_wait_code = 0;
	s_key_i = 0;
	pac->update = cheats_update;
}

void cheats_init(void)
{
	s_cheats_debug_mode = 0;
	s_cheats_demosave_mode = 0;
	s_cheats_translator_mode = 0;
}
