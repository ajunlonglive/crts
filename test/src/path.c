#define _XOPEN_SOURCE 500

#include <stdlib.h>
#include <time.h>

#include "client/graphics.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pg_node.h"
#include "server/sim/terrain.h"
#include "shared/math/perlin.h"
#include "shared/sim/chunk.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

#define RANGE 64

static struct point
random_point(void)
{
	struct point p = { random() % RANGE, random() % RANGE };

	return p;
}

struct point
find_random_point(struct chunks *cnks)
{
	size_t i = 0;
	struct point p = random_point();

	while (!is_traversable(cnks, &p)) {
		p.x++;

		if (++i > 256) {
			L("couldn't find a random point in time");
			exit(0);
		}
	}

	return p;
}

const char *seed0 = "0";

#define PEEPS 2
struct point peeps[PEEPS];

int
main(const int argv, const char **argc)
{
	const char *seed = argv > 1 ? argc[1] : seed0;
	unsigned int iseed = atoi(seed);
	int x;

	srandom(iseed);
	perlin_noise_shuf();

	struct chunks *cnks = NULL;

	chunks_init(&cnks);

	struct point pe = find_random_point(cnks);
	struct pgraph *pg = pgraph_create(cnks, &pe);

	for (x = 0; x < PEEPS; x++) {
		peeps[x] = find_random_point(cnks);
	}

	bool all_done = false;
	struct timespec start, stop;

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	while (!all_done) {
		all_done = true;
		for (x = 0; x < PEEPS; x++) {
			switch (pathfind(pg, &peeps[x])) {
			case rs_fail:
				return 1;
			case rs_cont:
				//L("%d, %d", peeps[x].x, peeps[x].y);
				all_done = false;
			/* FALLTHROUGH */
			case rs_done:
				break;
			}
		}
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);

	L("done in %ld secs, %ld ns", stop.tv_sec - start.tv_sec, stop.tv_nsec - start.tv_nsec);
	L("%ld nodes", darr_len(pg->heap));

	return 0;
}
