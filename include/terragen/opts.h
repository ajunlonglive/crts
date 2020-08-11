#ifndef TERRAGEN_OPTS_H
#define TERRAGEN_OPTS_H
#include "terragen/gen/gen.h"

struct cmdline_opts {
	struct terragen_opts opts;
	bool interactive;
};

void parse_cmdline_opts(int32_t argc, char *const *argv,
	struct cmdline_opts *opts);
#endif
