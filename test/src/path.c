#define _POSIX_C_SOURCE 199309L
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

#define ITS 256 * 16
#define DDIM 3

void
display_map(struct chunks *cnks, struct pgraph *g, struct point *ps, struct point *pe)
{
	struct point p = { 0, 0 };
	int i, j, x, y, cc = -1;
	size_t k, *ip;
	const struct chunk *cps[DDIM][DDIM * 2];
	struct pg_node *n;
	char c;

	for (i = 0; i < DDIM; p.y = ++i * CHUNK_SIZE) {
		for (j = 0; j < DDIM * 2; j++) {
			p.x = j * CHUNK_SIZE;
			cps[i][j] = get_chunk(cnks, &p);
		}
	}

	for (i = 0; i < DDIM; i++) {
		for (y = 0; y < CHUNK_SIZE; y++) {
			for (j = 0; j < DDIM * 2; j++) {
				for (x = 0; x < CHUNK_SIZE; x++) {
					cc = -1;

					p.x = (j * CHUNK_SIZE) + x;
					p.y = (i * CHUNK_SIZE) + y;

					// color chunk barriers
					if (x == 0 || y == 0) {
						cc = 46;
					}

					if (p.x == ps->x && p.y == ps->y) {
						c = '!';
						cc = 2;
					} else if (p.x == pe->x && p.y == pe->y) {
						c = 'E';
					} else {
						c = ' ';
					}

					if ((n = hdarr_get(g->nodes, &p)) == NULL) {
						cc = cc == -1 ? 40 + (int)cps[i][j]->tiles[x][y] : cc;
					} else {
						for (k = 0; k < darr_len(g->heap); k++) {
							ip = darr_get(g->heap, k);

							if (n == hdarr_get_by_i(g->nodes, *ip)) {
								cc = 47;
								break;
							}
						}
						c = c == ' ' ?
						    /*n->flow_calcd && n->flow.x != 0 ? n->flow.x < 0 ? 'l' : 'r'
						       : n->flow_calcd && n->flow.y != 0 ? n->flow.y < 0 ? 'u' : 'd'
						       :*/
						    (n->path_dist % 10) + '0' // path distance
						    : c;

						cc = cc == 0 ? 46 : cc;
					}

					printf("\033[2;30;%dm%c\033[%dm%c\033[0m",
						40 + cps[i][j]->tiles[x][y],
						graphics.tiles[cps[i][j]->tiles[x][y]].pix.c,
						cc,
						c);
				}
			}

			printf("\n");
		}
	}
}

static enum tile
tile_at_point(struct chunks *chnks, struct point *p)
{
	struct point np = nearest_chunk(p), rp = point_sub(p, &np);

	return get_chunk(chnks, &np)->tiles[rp.x][rp.y];
}

static struct point
random_point(void)
{
	struct point p = { random() % (DDIM * CHUNK_SIZE * 2), random() % (DDIM * CHUNK_SIZE) };

	return p;
}

struct point
find_random_point(struct chunks *cnks)
{
	struct point p = random_point();
	int i = 0;

	while (tile_at_point(cnks, &p) > tile_forest) {
		if (++i > 256) {
			L("couldn't find a random point in time");
			exit(0);
		}
		p = random_point();
	}

	return p;
}

const char *seed0 = "0";

#define PEEPS 1
struct point peeps[PEEPS];

int
main(const int argv, const char **argc)
{
	const char *seed = argv > 1 ? argc[1] : seed0;
	unsigned int iseed = atoi(seed);
	int x;

	srandom(iseed);
	//L("seed: %d, rand: %ld", iseed, random());
	perlin_noise_shuf();

	struct chunks *cnks = NULL;
	chunks_init(&cnks);
	struct point pe = find_random_point(cnks);
	struct pgraph *tpg = pgraph_create(cnks, &pe);
	int j, ret;

	for (x = 0; x < PEEPS; x++) {
		peeps[x] = find_random_point(cnks);
	}

	for (j = 0;; j++) {
		ret = 1;

		for (x = 0; x < PEEPS; x++) {
			ret &= pathfind(tpg, &peeps[x]);

			/*
			   if (j % 128 == 0) {
			        printf("\e[0;0H");
			        display_map(cnks, tpg, &peeps[x], &pe);
			        printf("(%s) pathfind it: %d,\e[K\n", seed, j);
			   }
			 */
		}
		if (ret == 1) {
			break;
		}
	}

	/*
	   printf("\e[0;0H");
	   display_map(cnks, tpg, &peeps[0], &pe);
	   printf("(%s) pathfind it: %d\e[K\n", seed, j);

	   L("done");
	 */
	return 0;
}
