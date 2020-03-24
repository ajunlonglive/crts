#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "client/opts.h"
#include "shared/util/log.h"

struct opts defaults = {
	.cfg = {
		.graphics = "cfg/graphics.ini",
		.keymap = "cfg/keymap.ini",
	},
	.ip_addr = "127.0.0.1",
};

static void
set_default_opts(struct opts *opts)
{
	srandom(time(NULL));
	*opts = defaults;
	opts->id = random();
}

void
process_opts(int argc, char * const *argv, struct opts *opts)
{
	signed char opt;

	set_default_opts(opts);

	while ((opt = getopt(argc, argv, "i:s:")) != -1) {
		switch (opt) {
		case 'i':
			opts->id = strtol(optarg, NULL, 10);
			break;
		case 's':
			strncpy(opts->ip_addr, optarg, 32);
			break;
		default:
			exit(EXIT_FAILURE);
			break;
		}
	}
}
