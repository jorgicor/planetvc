#include "coroutin.h"
#include <stddef.h>

static coroutine_t s_coroutine;

void start_coroutine(coroutine_t fn)
{
	if (fn(1) == 0) {
		s_coroutine = fn;
	}
}

int is_coroutine_running(void)
{
	return s_coroutine != NULL;
}

void run_coroutine(void)
{
	coroutine_t fn;

	fn = s_coroutine;
	s_coroutine = NULL;
	if (fn(0) == 0 && s_coroutine == NULL) {
		s_coroutine = fn;
	}
}

void coroutin_init(void)
{
	s_coroutine = NULL;
}
