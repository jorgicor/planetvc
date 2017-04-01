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

#include "state.h"
#include "cbase/kassert.h"
#include <stddef.h>

const struct state *g_state;

/*
 * This changes states immediately, so be careful with the code you have
 * after calling this. It is checked that this function is not called
 * recursively.
 */
void switch_to_state(const struct state *new_state)
{
	static int called = 0;
	const struct state *old;

	/*
	 * Check that we don't call this function recursively.
	 */
	kasserta(called == 0);
	called = 1;

	old = g_state;
	if (g_state != NULL && g_state->leave != NULL)
		g_state->leave(new_state);

	g_state = new_state;
	if (g_state != NULL && g_state->enter != NULL)
		g_state->enter(old);

	called = 0;
}

/*
 * Calls update() on the current state.
 * Then calls draw() on the current state after update() has been called.
 * Note that the state for update() can be different than the state on
 * draw() if switch_to_state() has been called in update().
 */
void update_state(void)
{
	if (g_state != NULL && g_state->update != NULL)
		g_state->update();
	if (g_state != NULL && g_state->draw != NULL)
		g_state->draw();
}

/*
 * This is a request. Will call end() on the state.
 * It's logical use is to request a state to end and switch
 * to another state.
 */
void end_state(void)
{
	if (g_state != NULL && g_state->end != NULL)
		g_state->end();
}

void state_init(void)
{
	g_state = NULL;
}
