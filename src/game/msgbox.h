/*
Copyright (c) 2014-2017 Jorge Giner Cordero

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
