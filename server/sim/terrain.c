#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "server/sim/terrain.h"
#include "shared/math/geom.h"
#include "shared/math/perlin.h"
#include "shared/sim/chunk.h"
#include "shared/sim/world.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define TPARAM_AMP   2.0f
#define TPARAM_FREQ  1.0f / 2.0f
#define TPARAM_OCTS  2
#define TPARAM_LACU  2.0f
#define TPARAM_BOOST TPARAM_AMP

static struct chunk *
full_init_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk c, *cp = &c;

	chunk_init(&cp);

	c.pos = *p;

	hdarr_set(cnks->hd, p, cp);

	return hdarr_get(cnks->hd, p);
}

static struct chunk *
get_chunk_no_gen(struct chunks *cnks, const struct point *p)
{
	const struct chunk *cnk;

	if ((cnk = hdarr_get(cnks->hd, p)) == NULL) {
		cnk = full_init_chunk(cnks, p);
	}

	return (struct chunk *)cnk;
}

static void
fill_chunk(struct chunk *a)
{
	int x, y;
	float fx, fy, fcs = (float)CHUNK_SIZE;
	int noise;

	for (y = 0; y < CHUNK_SIZE; y++) {
		for (x = 0; x < CHUNK_SIZE; x++) {
			fx = (float)(x + a->pos.x) / (fcs * 2.0);
			fy = (float)(y + a->pos.y) / (fcs * 1.0);

			noise = (int)roundf(
				perlin_two(
					fx,
					fy,
					TPARAM_AMP,
					TPARAM_OCTS,
					TPARAM_FREQ,
					TPARAM_LACU
					)
				) + TPARAM_BOOST;

			a->tiles[x][y] = noise < 0 ? 0 : (noise > TILE_MAX ? TILE_MAX : noise);
		}
	}

	a->empty = 0;
}

struct chunk *
get_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk *c = get_chunk_no_gen(cnks, p);

	if (c->empty) {
		fill_chunk(c);
	}

	return c;
}

bool
find_tile(enum tile t, struct chunks *cnks, struct circle *range, struct point *p)
{
	struct point q, r;

	for (p->x = range->center.x - range->r; p->x < range->center.x + range->r; ++p->x) {
		for (p->y = range->center.y - range->r; p->y < range->center.y + range->r; ++p->y) {
			if (!point_in_circle(p, range)) {
				continue;
			}

			q = nearest_chunk(p);
			r = point_sub(p, &q);

			if (get_chunk(cnks, &q)->tiles[r.x][r.y] == t) {
				return true;
			}
		}
	}

	return false;
}

bool
is_traversable(struct chunks *cnks, const struct point *p)
{
	struct point np = nearest_chunk(p), rp = point_sub(p, &np);

	if (get_chunk(cnks, &np)->tiles[rp.x][rp.y] <= tile_forest) {
		return true;
	} else {
		return false;
	}
}
