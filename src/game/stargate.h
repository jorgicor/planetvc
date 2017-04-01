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

#ifndef STARGATE_H
#define STARGATE_H

void stargate_init(void);

struct actor;

extern int s_active_stargate_id;

int stargate_check_teleport(struct actor *pac, struct actor *pcosmo, int oldx);
int stargate_get_goto_mapid(struct actor *pac);
int stargate_get_goto_gateid(struct actor *pac);
int stargate_get_next_active_gateid(struct actor *pac);
int stargate_get_id(struct actor *pac);
int stargate_get_arrow_dirpos(struct actor *pac);

void stargate_get_center(struct actor *pac, int *cx, int *cy);
void activate_stargate(struct actor *pac);
void deactivate_stargate(struct actor *pac);

#endif
