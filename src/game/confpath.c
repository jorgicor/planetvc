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

#include "confpath.h"
#include "gamelib/vfs.h"
#include "kernel/kernel.h"
#include "cfg/cfg.h"
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
	
	s_path[0] = '\0';

	cpath = kernel_get_device()->get_config_path(PP_ORG_NAME, PACKAGE);
	if (cpath == NULL)
		return;

	if (strlen(cpath) > OPEN_FILE_MAX_PATH_LEN)
		return;

	strcpy(s_path, cpath);
	free(cpath);
}
