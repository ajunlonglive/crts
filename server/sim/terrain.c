#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/math/geom.h"
#include "shared/math/perlin.h"
#include "shared/sim/chunk.h"
#include "shared/sim/world.h"
#include "shared/types/hash.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

#define TPARAM_AMP   2.0f
#define TPARAM_FREQ  1.0f / 2.0f
#define TPARAM_OCTS  3
#define TPARAM_LACU  2.0f
#define TPARAM_BOOST TPARAM_AMP

static struct chunk *
full_init_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk c, *cp = &c;

	chunk_init(&cp);

	c.pos = *p;
	c.last_touched = cnks->chunk_date;

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

struct chunk *
get_chunk_at(struct chunks *cnks, const struct point *p)
{
	struct point np = nearest_chunk(p);

	return get_chunk(cnks, &np);
}

bool
find_tile(enum tile t, struct chunks *cnks, const struct circle *range,
	const struct point *start, struct point *p, struct hash *skip)
{
	struct point q, r, c = { 0, 0 };
	uint32_t dist, cdist = UINT32_MAX;
	bool found = false;

	for (p->x = range->center.x - range->r; p->x < range->center.x + range->r; ++p->x) {
		for (p->y = range->center.y - range->r; p->y < range->center.y + range->r; ++p->y) {
			if (!point_in_circle(p, range)) {
				continue;
			}

			q = nearest_chunk(p);
			r = point_sub(p, &q);

			if (get_chunk(cnks, &q)->tiles[r.x][r.y] == t) {
				if (skip != NULL && hash_get(skip, p) != NULL) {
					continue;
				}

				found = true;
				dist = square_dist(start, p);
				if (dist < cdist) {
					cdist = dist;
					c = *p;
				}
			}
		}
	}

	*p = c;
	return found;
}

enum tile
get_tile_at(struct chunks *cnks, const struct point *p)
{
	struct chunk *ck = get_chunk_at(cnks, p);
	struct point rp = point_sub(p, &ck->pos);

	return ck->tiles[rp.x][rp.y];
}

bool
find_adj_tile(struct chunks *cnks, struct point *s, struct point *rp,
	struct circle *circ, enum tile t, bool (*pred)(enum tile t))
{
	enum tile tt;
	struct point p[4] = {
		{ s->x + 1, s->y     },
		{ s->x - 1, s->y     },
		{ s->x,     s->y + 1 },
		{ s->x,     s->y - 1 },
	};
	size_t i;

	for (i = 0; i < 4; ++i) {
		if (circ && !point_in_circle(&p[i], circ)) {
			continue;
		}

		tt = get_tile_at(cnks, &p[i]);

		if (tt == t || (pred && pred(tt))) {
			*rp = p[i];
			return true;
		}
	}

	return false;
}


bool
tile_is_traversable(enum tile t)
{
	return gcfg.tiles[t].traversable;
}

bool
is_traversable(struct chunks *cnks, const struct point *p)
{
	return tile_is_traversable(get_tile_at(cnks, p));
}

static void
commit_tile(struct chunks *cnks, const struct point *p, enum tile t)
{
	struct chunk *ck = get_chunk_at(cnks, p);
	struct point rp = point_sub(p, &ck->pos);

	if (t == ck->tiles[rp.x][rp.y]) {
		return;
	}

	if (gcfg.tiles[ck->tiles[rp.x][rp.y]].functional) {
		hash_unset(cnks->functional_tiles, p);
	}

	ck->tiles[rp.x][rp.y] = t;
	ck->harvested[rp.x][rp.y] = 0;
	ck->last_touched = ++cnks->chunk_date;
	ck->touched_this_tick |= true;
}

void
update_tile(struct chunks *cnks, const struct point *p, enum tile t)
{

	assert(!gcfg.tiles[t].functional);

	commit_tile(cnks, p, t);

}

void
update_functional_tile(struct chunks *cnks, const struct point *p, enum tile t,
	uint16_t mot)
{
	assert(gcfg.tiles[t].functional);

	commit_tile(cnks, p, t);

	union functional_tile ft = { .ft = { .type = t, .motivator = mot } };

	hash_set(cnks->functional_tiles, p, ft.val);
}
