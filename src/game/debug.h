/*
 * Copyright 2016 Jorge Giner Cordero
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "config.h"

enum {

#ifdef PP_DEBUG_ON
	DEBUG_ON = 1,
#else
	DEBUG_ON = 0,
#endif

#ifdef PP_DEMO_ON
	DEMO_ON = 1,
#else
	DEMO_ON = 0,
#endif

};

#endif
