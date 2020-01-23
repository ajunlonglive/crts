#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#include <time.h>
#include <stdlib.h>

#include "util/log.h"
#include "math/perlin.h"
#include "sim/chunk.h"
#include "../../server/src/sim/terrain.h"
#include "../../server/src/sim/pathfind/mapping.h"
#include "constants/tile_chars.h"
#include "../../server/src/sim/pathfind/pathfind.h"

#define ITS 256 * 16
#define DDIM 5

static void
display_map(struct chunks *cnks, struct path_graph *g, struct point *ps, struct point *pe)
{
	struct point p = { 0, 0 };
	int i, j, x, y, cc = -1, k;
	const struct chunk *cps[DDIM][DDIM * 2];
	struct node *n;
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

					if ((n = pgraph_lookup(g, &p)) == NULL) {
						cc = cc == -1 ? 40 + (int)cps[i][j]->tiles[x][y] : cc;
					} else {
						for (k = 0; k < (int)g->heap.len; k++) {
							if (n == g->nodes.e + g->heap.e[k]) {
								cc = 47;
								break;
							}
						}
						c = c == ' ' ?
						    /*n->flow_calcd && n->flow.x != 0 ? n->flow.x < 0 ? 'l' : 'r'
						       : n->flow_calcd && n->flow.y != 0 ? n->flow.y < 0 ? 'u' : 'd'
						       :*/
						    (n->trav > 9 ? 'a' + n->trav % 10 : '0' + n->trav)     // traversability
						    //(n->path_dist % 10) + '0' // path distance
						    : c;

						if (n->flow_calcd) {
							cc = 45;
						}

						cc = cc == 0 ? 46 : cc;
					}

					printf("\033[2;30;%dm%c\033[%dm%c\033[0m",
						40 + cps[i][j]->tiles[x][y],
						tile_chars[cps[i][j]->tiles[x][y]],
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

#define PEEPS 128
struct point peeps[PEEPS];

int
main(const int argv, const char **argc)
{
	const char *seed = argv > 1 ? argc[1] : seed0;
	int x;
	//int use_hgraph = 1;

	srand(argv > 1 ? atoi(argc[1]) : 0);
	if (argv > 2) {
		switch (argc[2][0]) {
		case 'H':
			//use_hgraph = 0;
			break;
		}
	}

	perlin_noise_shuf();

	struct chunks *cnks = NULL;
	chunks_init(&cnks);
	struct point pe = find_random_point(cnks);
	struct path_graph *tpg = tile_pg_create(cnks, &pe);
	//                              999999999.
	//const struct timespec ts = { 0, 500000 };
	int j, ret;

	for (x = 0; x < PEEPS; x++) {
		peeps[x] = find_random_point(cnks);
	}

	/*
	 * (5226) pathfind it: 15, 1311
	   peeps[0].x = (16 * 6) + 5;
	   peeps[0].y = (16 * 4) + 5;
	 */

	for (j = 0;; j++) {
		ret = 1;

		for (x = 0; x < PEEPS; x++) {
			ret &= pathfind(tpg, &peeps[x]);

			if (j % 128 == 0) {
				printf("\e[0;0H");
				display_map(cnks, tpg, &peeps[x], &pe);
				printf("(%s) pathfind it: %d,\e[K\n", seed, j);
			}
		}
		if (ret == 1) {
			break;
		}
	}

	printf("\e[0;0H");
	display_map(cnks, tpg, &peeps[0], &pe);
	printf("(%s) pathfind it: %d\e[K\n", seed, j);

	L("done");
	return 0;
}
