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

#include "input.h"
#include "kernel/kernel.h"
#include "cbase/cbase.h"
#include "cbase/kassert.h"
#include <string.h>

/* Maps virtual keys to kernel scancodes. */
static const int s_keys_const[GAME_NKEYS] = {
	-1,
	KERNEL_KSC_DOWN,
	KERNEL_KSC_LEFT,
	KERNEL_KSC_RIGHT,
	KERNEL_KSC_UP,
	KERNEL_KSC_SPACE,
	KERNEL_KSC_0,
	-1,
	KERNEL_KSC_UP,
	KERNEL_KSC_DOWN,
	KERNEL_KSC_LEFT,
	KERNEL_KSC_RIGHT,
	KERNEL_KSC_RETURN,
	-1,
	KERNEL_KSC_ESC,
	KERNEL_KSC_ESC
};

static int s_keys[NELEMS(s_keys_const)];

/* Maps joystick 0 scancodes to virtual keys. */
static const struct pad_vkeys {
	int key;
	int lkey;
} s_buttons[KERNEL_NPAD_BUTTONS] = {
	{ KEYA, LKEYA },	/* A */
	{ KEYB, LKEYB },	/* B */
	{ KEYX, LKEYX },	/* X */
	{ KEYY, LKEYY },	/* Y */
	{ -1, -1 },		/* BACK */
	{ -1, -1 },		/* GUIDE */
	{ -1, LKEYY },		/* START */
	{ -1, -1 },		/* LSTICK */
	{ -1, -1 },		/* RSTICK */
	{ KDOWN, -1 },		/* LSHOULDER */
	{ KDOWN, -1 },		/* RSHOULDER */
	{ KUP, LKUP },		/* DUP */
	{ KDOWN, LKDOWN },	/* DDOWN */
	{ KLEFT, LKLEFT },	/* DLEFT */
	{ KRIGHT, LKRIGHT },	/* DRIGHT */
};

enum {
	AXIS_LIMIT = 8000
};

static struct axis_dir {
	int axis;
	int dir;
} s_axis_dir[GAME_NKEYS] = {
	{ KERNEL_AXIS_V1, -1 },
	{ KERNEL_AXIS_V1, 1 },
	{ KERNEL_AXIS_H1, -1 },
	{ KERNEL_AXIS_H1, 1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ KERNEL_AXIS_V1, -1 },
	{ KERNEL_AXIS_V1, 1 },
	{ KERNEL_AXIS_H1, -1 },
	{ KERNEL_AXIS_H1, 1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
	{ -1, -1 },
};

/* Redefine a game key to a kernel scan code.
 * Meant to be used with: 
 *	KUP, KDOWN, KLEFT, KRIGHT,
 *     	KEYA, KEYB, KEYX, KEYY .
 */
void redefine_key(int game_key, int ksc)
{
	if (game_key < 0 || game_key > KEYY)
		return;

	if (ksc < 0 || ksc >= KERNEL_KSC_PAD0_A)
		return;

	s_keys[game_key] = ksc;
}

/* Returns the kernel value of a game key.
 * Returns -1 if game_key is not
 *	KUP, KDOWN, KLEFT, KRIGHT,
 *     	KEYA, KEYB, KEYX, KEYY .
 */
int get_game_key_value(int game_key)
{
	if (kassert_fails(game_key >= 0 && game_key <= KEYY)) {
		return -1;
	}
	return s_keys[game_key];
}

int is_key_down(int game_key)
{
	const struct kernel_device *kdev;
	int i, down, val, j;

	if (game_key < 0 || game_key >= GAME_NKEYS)
		return 0;

	down = 0;
	kdev = kernel_get_device();
	for (i = 0; !down && i < KERNEL_NPADS; i++) {
		for (j = 0; !down && j < NELEMS(s_buttons); j++) {
			if (s_buttons[j].key == game_key ||
				s_buttons[j].lkey == game_key)
			{
				down |= kdev->key_down(
					KERNEL_KSC_PAD0_A + j +
					i * KERNEL_NPAD_BUTTONS);
			}
		}
	}

	/* See analog axis */
	if (!down && s_axis_dir[game_key].axis != -1) {
		for (i = 0; !down && i < KERNEL_NPADS; i++) {
			val = kdev->get_axis_value(i,
				s_axis_dir[game_key].axis);
			if (s_axis_dir[game_key].dir > 0)
				down = val > AXIS_LIMIT;
			else
				down = val < -AXIS_LIMIT;
		}
	}

	if (!down) {
		down |= kdev->key_down(s_keys[game_key]);
	}

	return down;
}

int is_first_pressed(int game_key)
{
	const struct kernel_device *kdev;
	int i, down, j;

	if (game_key < 0 || game_key >= GAME_NKEYS)
		return 0;

	down = 0;
	kdev = kernel_get_device();
	for (i = 0; !down && i < KERNEL_NPADS; i++) {
		for (j = 0; !down && j < NELEMS(s_buttons); j++) {
			if (s_buttons[j].key == game_key ||
				s_buttons[j].lkey == game_key)
			{
				down |= kdev->key_first_pressed(
					KERNEL_KSC_PAD0_A + j +
					i * KERNEL_NPAD_BUTTONS);
			}
		}
	}

	if (!down) {
		down |= kdev->key_first_pressed(s_keys[game_key]);
	}

	return down;
}

void input_init(void)
{
	memcpy(s_keys, s_keys_const, sizeof(s_keys));
}
