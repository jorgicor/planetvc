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

#include "menu.h"
#include "text.h"
#include "strdraw.h"
#include "tilengin.h"
#include "input.h"
#include "kernel/kernel.h"
#include "cbase/kassert.h"

#ifndef STRING_H
#define STRING_H
#include <string.h>
#endif

enum {
	NMENUS = 5
};

struct menu_info {
	const struct menu *pmenu;
	int n, opi;
	int x, y;
	int enabled;
};

static struct menu_info s_menus[NMENUS];
static int s_free_index;

static void draw_arrow(void)
{
	int y;
	struct menu_info *pminfo;

	if (s_free_index == 0)
		return;

	pminfo = &s_menus[s_free_index - 1];
	y = pminfo->y + (pminfo->opi * 2);
	te_set_fg_xy(pminfo->x - 2, y, 0, chri(CHR_ARROW_R)); 
}

static void clear_arrow(void)
{
	int y;
	struct menu_info *pminfo;

	if (s_free_index == 0)
		return;

	pminfo = &s_menus[s_free_index - 1];
	y = pminfo->y + (pminfo->opi * 2);
	te_set_fg_xy(pminfo->x - 2, y, 0, chri(' ')); 
}

static void menu_clear(void)
{
	struct menu_info *pminfo;
	int i, y;

	if (s_free_index == 0) {
		return;
	}

	pminfo = &s_menus[s_free_index - 1];
	y = pminfo->y;
	for (i = 0; i < MENU_NOPTIONS; i++) {
		if (pminfo->pmenu->options[i].str == NULL) {
			break;
		}
		te_fill_fg(0, y, TE_FMW, 2, 0, chri(' '));
		y += 2;
	}

	clear_arrow();
}

void menu_pop(void)
{
	if (kassert_fails(s_free_index > 0))
		return;

	menu_clear();
	s_free_index--;
	if (s_free_index > 0) {
		menu_redraw();
	}
}

static void menu_next_option(void)
{
	struct menu_info *pminfo;

	if (kassert_fails(s_free_index > 0))
		return;

	pminfo = &s_menus[s_free_index - 1];
	clear_arrow();
	pminfo->opi = (pminfo->opi + 1) % pminfo->n;
	draw_arrow();
}

static void menu_prev_option(void)
{
	struct menu_info *pminfo;

	if (kassert_fails(s_free_index > 0))
		return;

	pminfo = &s_menus[s_free_index - 1];
	clear_arrow();
	if (pminfo->opi == 0)
		pminfo->opi = pminfo->n - 1;
	else
		pminfo->opi--;
	draw_arrow();
}

/* Returns -1 if no option selected.
 * -2 if no option selected but the cursor moved.
 * -3 if the ESC pressed (you can go back with menu_pop).
 */
int menu_update(void)
{
	const struct kernel_device *d;
	struct menu_info *pminfo;

	if (s_free_index == 0)
		return -1;

	pminfo = &s_menus[s_free_index - 1];
	if (!pminfo->enabled)
		return -1;

	d = kernel_get_device();
	if (d->key_first_pressed(KERNEL_KSC_ESC) && s_free_index > 0) {
		return -3;
	} else if (is_first_pressed(LKDOWN) ||
		d->key_first_pressed(KERNEL_KSC_SPACE))
       	{
		menu_next_option();
		return -2;
	} else if (is_first_pressed(LKUP)) {
		menu_prev_option();
		return -2;
	} else if (is_first_pressed(LKEYA)) {
		return pminfo->pmenu->options[pminfo->opi].code;
	}

	return -1;
}

/* Returns the y position of the current selected option. */
int menu_get_cur_op_y(void)
{
	struct menu_info *pminfo;

	if (s_free_index == 0)
		return -1;

	pminfo = &s_menus[s_free_index - 1];
	return pminfo->y + pminfo->opi * 2;
}

int menu_get_x(void)
{
	if (s_free_index == 0)
		return 0;

	return s_menus[s_free_index - 1].x;
}

/* Returns the index of the current selected option. */
int menu_get_cur_op_i(void)
{
	if (s_free_index == 0)
		return -1;

	return s_menus[s_free_index - 1].opi;
}

void menu_redraw(void)
{
	struct menu_info *pminfo;
	int i, y, len;
	const char *str;

	if (s_free_index == 0)
		return;

	pminfo = &s_menus[s_free_index - 1];
	y = pminfo->y;
	for (i = 0; i < MENU_NOPTIONS; i++) {
		if (pminfo->pmenu->options[i].str == NULL) {
			break;
		}
		te_fill_fg(0, y, TE_FMW, 2, 0, chri(' '));
		str = _(pminfo->pmenu->options[i].str);
		len = utf8_strlen(str);
		draw_str(str, pminfo->x, y, 0);
		if (pminfo->pmenu->options[i].icon != NULL) {
			draw_icon(pminfo->pmenu->options[i].icon,
				  pminfo->x + len + 1, y, 0);
		}
		y += 2;
	}

	draw_arrow();
	if (!pminfo->enabled)
		clear_arrow();
}

/* If b != 1, Hides arrow and disables responding to keys.
 * Else shows arrow, and enables responding to keys.
 */
void menu_enable(int b)
{
	if (s_free_index == 0)
		return;

	s_menus[s_free_index - 1].enabled = b;
	if (b)
		draw_arrow();
	else
		clear_arrow();
}

static int menu_calc_x(const struct menu *pmenu, const char **additional)
{
	int i, len, maxlen;

	maxlen = 0;
	for (i = 0; i < MENU_NOPTIONS; i++) {
		if (pmenu->options[i].str == NULL)
			break;
		len = utf8_strlen(_(pmenu->options[i].str));
		if (len > maxlen)
			maxlen = len;
	}

	if (additional != NULL) {
		for (i = 0; i < MENU_NADDITIONAL; i++) {
			if (additional[i] == NULL)
				break;
			len = utf8_strlen(_(additional[i]));
			if (len > maxlen)
				maxlen = len;
		}
	}

	return (TE_FMW - maxlen) / 2;
}

static int menu_count_options(const struct menu *pmenu)
{
	int i;

	for (i = 0; i < MENU_NOPTIONS; i++) {
		if (pmenu->options[i].str == NULL)
			break;
	}

	return i;
}

/* Shows a menu at y position.
 * 'firstop' is code of the first option to mark with arrow, -1 can be passed
 * to select the first option.
 * 'additional' contains other options that may appear in the menu
 * to calculate its centered x position; it must be NULL terminated
 * and can be NULL.
 */
void menu_push(const struct menu *pmenu, int y, int firstop,
	       const char **additional)
{
	int i;
	struct menu_info *pminfo;

	if (kassert_fails(s_free_index < NMENUS))
		return;

	if (s_free_index > 0) {
		menu_clear();
	}

	pminfo = &s_menus[s_free_index];
	pminfo->pmenu  = pmenu;
	pminfo->y  = y;
	pminfo->x = menu_calc_x(pmenu, additional);
	pminfo->n = menu_count_options(pmenu);
	pminfo->enabled = 1;

	pminfo->opi = 0;
	if (firstop >= 0) {
		for (i = 0; i < pminfo->n; i++) {
			if (firstop == pminfo->pmenu->options[i].code) {
				pminfo->opi = i;
				break;
			}
		}
	}

	s_free_index++;
	menu_redraw();
}

void menu_reset_stack(void)
{
	s_free_index = 0;
}

int menu_stack_count(void)
{
	return s_free_index;
}

void menu_init(void)
{
	menu_reset_stack();
}
