#ifndef COROUTIN_H
#define COROUTIN_H

#define crBegin(restart) \
	static int state=0; \
	state = restart ? 0 : state; \
	switch (state) { case 0:

#define crReturn(x) do { \
	state = __LINE__; return x; \
	case __LINE__:; } while (0)

#define crFinish }		      

/* Coroutines must return != 0 to signal that they ended. */
typedef int (*coroutine_t)(int restart);

void start_coroutine(coroutine_t fn);
int is_coroutine_running(void);
void run_coroutine(void);

void coroutin_init(void);

#endif
