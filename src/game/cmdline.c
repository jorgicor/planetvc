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

#include "cmdline.h"
#include "gamelib/ngetopt.h"
#include "cbase/kassert.h"
#include <stdlib.h>

int s_cmdline_filter = 1;
int s_cmdline_scale = 0;

void read_cmd_line(int argc, char *argv[])
{
	static struct ngetopt_opt ops[] = {
		{ "filter", 1, 'f' },
		{ "scale", 1, 's' },
		{ "use-my-sdl", 0, 0 },
		{ "m32", 0, 0 },
		{ "m64", 0, 0 },
		{ NULL, 0, 0 },
	};

	char c;
	struct ngetopt ngo;

	ngetopt_init(&ngo, argc, argv, ops);
	do {
		c = ngetopt_next(&ngo);
		switch (c) {
		case 'f':
			s_cmdline_filter = atoi(ngo.optarg);
			break;
		case 's':
			s_cmdline_scale = atoi(ngo.optarg);
			break;
		case '?':
			ktrace("unrecognized option %s", ngo.optarg);
			break;
		case ':':
			ktrace("the -%c option needs an argument",
				(char) ngo.optopt);
			break;
		}
	} while (c != -1);
}
