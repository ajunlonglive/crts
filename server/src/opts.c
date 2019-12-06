#define _XOPEN_SOURCE 500
#include <stdlib.h>

#include "opts.h"
#include "math/perlin.h"
#include "util/log.h"

void process_opts(int argc, const char **argv)
{
	if (argc < 2) {
		L("error: please provide a seed");
		exit(1);
	} else {
		srandom(atoi(argv[1]));
		perlin_noise_shuf();
	}
}
