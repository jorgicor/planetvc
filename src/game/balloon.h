#ifndef BALLOON_H
#define BALLOON_H

struct frame;

void show_balloon(struct frame *icon);
void hide_balloon(void);

void balloon_init(void);

#endif
