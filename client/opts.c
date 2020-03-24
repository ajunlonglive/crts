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

static void
print_usage(void)
{
	printf("usage: crts [OPTIONS]\n"
		"\n"
		"OPTIONS:\n"
		"-g <path>         - set graphics cfg\n"
		"-k <path>         - set keymap cfg\n"
		"-i <integer>      - set client id\n"
		"-s <ip address>   - set server ip\n"
		"-h                - show this message\n"
		);
}

void
process_opts(int argc, char * const *argv, struct opts *opts)
{
	signed char opt;

	set_default_opts(opts);

	while ((opt = getopt(argc, argv, "g:hi:k:s:")) != -1) {
		switch (opt) {
		case 'g':
			strncpy(opts->cfg.graphics, optarg, OPT_STR_VALUE_LEN - 1);
			break;
		case 'h':
			print_usage();
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			opts->id = strtol(optarg, NULL, 10);
			break;
		case 'k':
			strncpy(opts->cfg.keymap, optarg, OPT_STR_VALUE_LEN - 1);
			break;
		case 's':
			strncpy(opts->ip_addr, optarg, OPT_STR_VALUE_LEN - 1);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
			break;
		}
	}
}
