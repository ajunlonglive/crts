#ifndef TERRAGEN_TERRAGEN_H
#define TERRAGEN_TERRAGEN_H

#include <stdbool.h>

#include "terragen/gen/opts.h"

struct terragen_opts {
	terragen_opts opts;
	const char *world_file;
	bool interactive;
};

void terragen_main(struct terragen_opts *opts);
void parse_terragen_opts(int argc, char * const *argv, struct terragen_opts *opts);
#endif
