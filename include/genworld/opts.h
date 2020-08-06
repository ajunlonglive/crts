#ifndef SERVER_WORLDGEN_OPTS_H
#define SERVER_WORLDGEN_OPTS_H
#include "genworld/gen.h"

struct genworld_opts {
	struct worldgen_opts opts;
	bool interactive;
};

void parse_cmdline_opts(int32_t argc, char *const *argv, struct genworld_opts *opts);
#endif
