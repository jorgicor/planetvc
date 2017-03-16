/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef MSGBOX_H
#define MSGBOX_H

/*
 * IDLE a msgbox is running but nothing selected.
 * BACK a msgbox with can_go_back and back pressed.
 * NONE there is no msgbox runing.
 */
enum {
	MSGBOX_IDLE = -1,
	MSGBOX_BACK = -2,
	MSGBOX_NONE = -3
};

struct msgbox_option {
	const char *str;
	int code;
};

/* For the icon format see draw_icon() in strdraw. */
struct msgbox {
	const char *icon;
	const char *title;
	struct msgbox_option options[4];
	int x, w;
	int can_go_back;
};

extern struct wav *wav_opsel;
extern struct wav *wav_opmove;

void show_msgbox(const struct msgbox *mbox);
int update_msgbox(void);

void msgbox_init(void);

#endif
