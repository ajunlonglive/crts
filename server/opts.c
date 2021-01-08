#include "posix.h"

#include <stdlib.h>

#include "server/opts.h"
#include "shared/util/log.h"

static void
print_usage(void)
{
	printf("usage: server [opts]\n"
		"\n"
		"opts:\n"
		"  -h                      - show this message\n"
		);
}

void
parse_server_opts(int argc, char *const *argv, struct server_opts *so)
{
	signed char opt;

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
}
