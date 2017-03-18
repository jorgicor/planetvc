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
