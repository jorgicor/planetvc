#ifndef MENU_H
#define MENU_H

enum {
	MENU_NOPTIONS = 8,
	MENU_NADDITIONAL = 8
};

struct menu_option {
	const char *str;
	int code;
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

#endif