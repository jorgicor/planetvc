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

#ifndef MENU_H
#define MENU_H

enum {
	MENU_NOPTIONS = 8,
	MENU_NADDITIONAL = 8
};

/* For the icon string, see draw_icon in strdraw. */
struct menu_option {
	const char *str;
	int code;
	const char *icon;
};

/* Should have NOPTIONS or be NULL terminated, that is,
 * the last option should be { NULL, whatever }.
 */
struct menu {
	struct menu_option options[MENU_NOPTIONS];
};

void menu_push(const struct menu *pmenu, int y, int firstop,
	       const char **additional);
void menu_enable(int b);
void menu_redraw(void);
int menu_update(void);
int menu_get_cur_op_i(void);
int menu_get_cur_op_y(void);
int menu_get_x(void);
void menu_pop(void);
void menu_reset_stack(void);
int menu_stack_count(void);

void menu_init(void);

#endif
