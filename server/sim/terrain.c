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

static uint32_t
determine_grow_chance(struct chunk *ck, int32_t x, int32_t y, enum tile t)
{
	struct point p[4] = {
		{ x + 1, y     },
		{ x - 1, y     },
		{ x,     y + 1 },
		{ x,     y - 1 },
	};
	uint8_t adj = 0;
	size_t i;

	enum tile trigger = trigger = gcfg.tiles[t].next_to;

	trigger ? : (trigger = t);

	for (i = 0; i < 4; ++i) {
		if (p[i].x < 0 || p[i].x >= CHUNK_SIZE || p[i].y < 0 || p[i].y >= CHUNK_SIZE) {
			continue;
		}

		if (trigger == ck->tiles[p[i].x][p[i].y]) {
			++adj;
		}
	}

	return adj > 0 ? gcfg.misc.terrain_base_adj_grow_chance / adj
		: gcfg.misc.terrain_base_not_adj_grow_chance;
}

bool
age_chunk(struct chunk *ck)
{
	enum tile t, nt;
	struct point c;
	uint32_t chance;
	bool updated = false;

	for (c.x = 0; c.x < CHUNK_SIZE; ++c.x) {
		for (c.y = 0; c.y < CHUNK_SIZE; ++c.y) {
			t = ck->tiles[c.x][c.y];

			if (!(nt = gcfg.tiles[t].next)) {
				continue;
			}

			chance = determine_grow_chance(ck, c.x, c.y, t);

			if (chance == 0 || random() % chance != 0) {
				continue;
			}

			ck->tiles[c.x][c.y] = gcfg.tiles[t].next;
			updated = true;
		}
	}

	return updated;
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

	y = gcfg.misc.terrain_initial_age_multiplier *
	    (random() % gcfg.misc.terrain_initial_age_max);

	for (x = 0; x < y; ++x) {
		age_chunk(a);
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
	struct circle *circ, enum tile t, enum ent_type et,
	bool (*pred)(enum tile t, enum ent_type et))
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

		if (tt == t || (pred && pred(tt, et))) {
			*rp = p[i];
			return true;
		}
	}

	return false;
}


bool
tile_is_traversable(enum tile t, enum ent_type et)
{
	return gcfg.tiles[t].trav_type & gcfg.ents[et].trav;
}

bool
is_traversable(struct chunks *cnks, const struct point *p, enum ent_type t)
{
	return tile_is_traversable(get_tile_at(cnks, p), t);
}

void
touch_chunk(struct chunks *cnks, struct chunk *ck)
{
	ck->last_touched = ++cnks->chunk_date;
	ck->touched_this_tick |= true;
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

	touch_chunk(cnks, ck);
}

void
update_tile(struct chunks *cnks, const struct point *p, enum tile t)
{

	assert(!gcfg.tiles[t].functional);

	commit_tile(cnks, p, t);

}

void
update_functional_tile(struct chunks *cnks, const struct point *p, enum tile t,
	uint16_t mot, uint32_t tick)
{
	assert(gcfg.tiles[t].functional);

	commit_tile(cnks, p, t);

	union functional_tile ft = { .ft = { .type = t, .motivator = mot,
					     .tick = tick } };

	hash_set(cnks->functional_tiles, p, ft.val);
}
