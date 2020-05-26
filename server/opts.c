#include <stdlib.h>

#include "server/opts.h"
#include "shared/math/perlin.h"
#include "shared/util/log.h"

void
process_opts(int argc, const char **argv)
{
	if (argc < 2) {
		L("error: please provide a seed");
		exit(1);
	} else {
		srand(atoi(argv[1]));
		perlin_noise_shuf();
	}
}
