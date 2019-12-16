#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#include <time.h>
#include <stdlib.h>

#include "util/log.h"
#include "math/perlin.h"
#include "sim/chunk.h"
#include "../../server/src/sim/terrain.h"
#include "constants/tile_chars.h"
#include "../../server/src/sim/pathfind.h"

#define ITS 256 * 16
#define DDIM 3

static void display_map(struct hash *cnks, struct graph *g, struct point *ps, struct point *pe)
{
	struct point p = { 0, 0 };
	int i, j, x, y, cc, k;
	struct chunk *cps[DDIM][DDIM * 2];
	struct node *n;
	char c;

	for (i = 0; i < DDIM; p.y = ++i * CHUNK_SIZE)
		for (j = 0; j < DDIM * 2; j++) {
			p.x = j * CHUNK_SIZE;
			cps[i][j] = get_chunk(cnks, &p);
		}

	for (i = 0; i < DDIM; i++)
		for (y = 0; y < CHUNK_SIZE; y++) {
			for (j = 0; j < DDIM * 2; j++)
				for (x = 0; x < CHUNK_SIZE; x++) {
					p.x = (j * CHUNK_SIZE) + x;
					p.y = (i * CHUNK_SIZE) + y;

					if (p.x == ps->x && p.y == ps->y) {
						c = '@';
						cc = 43;
					} else if (p.x == pe->x && p.y == pe->y) {
						c = 'E';
					} else {
						c = ' ';
					}

					cc = 0;
					if ((n = graph_get(g, &p)) == NULL) {
						cc = 40 + cps[i][j]->tiles[x][y];
					} else {
						for (k = 0; k < (int)g->fringe.len; k++)
							if (n == g->nodes.e + g->fringe.e[k]) {
								cc = 47;
								break;
							}
						c = c == ' ' ?
						    /*n->flow_calcd && n->flow.x != 0 ? n->flow.x < 0 ? 'l' : 'r'
						       : n->flow_calcd && n->flow.y != 0 ? n->flow.y < 0 ? 'u' : 'd'
						       :*/'0' + n->path_dist % 10 : c;

						if (n->flow_calcd)
							cc = 45;

						cc = cc == 0 ? 46 : cc;
					}

					printf("\033[2;30;%dm%c\033[%dm%c\033[0m",
					       40 + cps[i][j]->tiles[x][y],
					       tile_chars[cps[i][j]->tiles[x][y]],
					       cc,
					       c);
				}

			printf("\n");
		}
}

static enum tile tile_at_point(struct hash *chnks, struct point *p)
{
	struct point np = nearest_chunk(p), rp = point_sub(p, &np);

	return get_chunk(chnks, &np)->tiles[rp.x][rp.y];
}

static struct point random_point()
{
	struct point p = { random() % (DDIM * CHUNK_SIZE * 2), random() % (DDIM * CHUNK_SIZE) };

	return p;
}

struct point find_random_point(struct hash *cnks)
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

#define PEEPS 20
struct point peeps[PEEPS];

int main(const int argv, const char **argc)
{
	const char *seed = argv > 1 ? argc[1] : seed0;
	int x;

	srand(argv > 1 ? atoi(argc[1]) : 0);
	perlin_noise_shuf();

	struct hash *cnks = hash_init(sizeof(struct point));
	struct point ps, pe = find_random_point(cnks);
	struct graph *g = create_graph(cnks, &pe);
	//                              999999999.
	//const struct timespec ts = { 0, 500000 };
	int i, ret;

	for (x = 0; x < PEEPS; x++)
		peeps[x] = find_random_point(cnks);

	for (i = 0;; i++) {
		ret = 1;

		for (x = 0; x < PEEPS; x++) {
			ret &= pathfind(cnks, g, &peeps[x]);

			printf("\e[0;0H");
			display_map(cnks, g, &ps, &pe);
			printf("(%s) pathfind it: %d\e[K\n", seed, i);
		}

		if (ret)
			break;
	}

	L("(%d, %d)", ps.x, ps.y);

	L("done");
	return 0;
}
