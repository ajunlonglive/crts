#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include "posix.h"

#include <stdlib.h>
#include <time.h>

#include "client/ui/ncurses/graphics.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/pathfind/pg_node.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/math/perlin.h"
#include "shared/sim/chunk.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

#define SEED 1234
#define RANGE 64
#define PEEPS 2

struct point peeps[PEEPS];

static struct point
random_point(void)
{
	struct point p = { rand() % RANGE, rand() % RANGE };

	return p;
}

struct point
find_random_point(struct chunks *cnks)
{
	size_t i = 0;
	struct point p = random_point();

	while (!is_traversable(cnks, &p, et_worker)) {
		p.x++;

		if (++i > 256) {
			L("couldn't find a random point in time");
			exit(0);
		}
	}

	return p;
}

int
main(const int argv, const char **argc)
{
	bool all_done = false;
	int x;
	struct chunks *cnks = NULL;
	struct pgraph *pg;
	struct point pe;
	struct timespec start, stop;

	srand(SEED);
	perlin_noise_shuf();

	chunks_init(&cnks);

	pe = find_random_point(cnks);
	pg = pgraph_create(cnks, &pe, trav_land);

	for (x = 0; x < PEEPS; x++) {
		peeps[x] = find_random_point(cnks);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	while (!all_done) {
		all_done = true;
		for (x = 0; x < PEEPS; x++) {
			switch (pathfind(pg, &peeps[x])) {
			case rs_fail:
				goto finished;
			case rs_cont:
				all_done = false;
			/* FALLTHROUGH */
			case rs_done:
				break;
			}
		}
	}

finished:
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);

	L("done: %d | in %ld secs, %ld nodes", all_done,
		stop.tv_sec - start.tv_sec, darr_len(pg->heap));

	pgraph_destroy(pg);
	free(pg);
	chunks_destroy(cnks);

	return 0;
}
