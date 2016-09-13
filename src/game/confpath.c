/*
 * Copyright 2016 Jorge Giner Cordero
 */

#include "confpath.h"
#include "gamelib/vfs.h"
#include "kernel/kernel.h"
#include "config.h"
#include <string.h>

#ifndef STDLIB_H
#define STDLIB_H
#include <stdlib.h>
#endif

static char s_path[OPEN_FILE_MAX_PATH_LEN + 1];

const char *confpath_get(void)
{
	return s_path;
}

void confpath_init(void)
{
	char *cpath;
	
	cpath = kernel_get_device()->get_config_path(PP_ORG_NAME, PACKAGE);
	if (cpath == NULL)
		return;

	if (strlen(cpath) > OPEN_FILE_MAX_PATH_LEN)
		return;

	strcpy(s_path, cpath);
	free(cpath);
}
