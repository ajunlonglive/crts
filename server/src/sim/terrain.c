#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <math.h>

#include "math/perlin.h"
#include "util/log.h"
#include "util/mem.h"
#include "sim/chunk.h"
#include "sim/world.h"
#include "math/geom.h"
#include "types/hash.h"
#include "terrain.h"

#define TPARAM_AMP   2.0f
#define TPARAM_FREQ  1.0f / 2.0f
#define TPARAM_OCTS  2
#define TPARAM_LACU  2.0f
#define TPARAM_BOOST TPARAM_AMP

static unsigned full_init_chunk(struct chunks *cnks, const struct point *p)
{
	union {
		void **vp;
		struct chunk **cp;
	} cp = { .cp = &cnks->mem.e };

	unsigned off = get_mem(cp.vp, sizeof(struct chunk), &cnks->mem.len, &cnks->mem.cap);
	struct chunk *c = cnks->mem.e + off;

	chunk_init(&c);
	c->pos = *p;

	return off;
}

static const struct chunk *get_chunk_no_gen(struct chunks *cnks, const struct point *p)
{
	unsigned c;
	const struct hash_elem *he;

	if ((he = hash_get(cnks->h, p)) == NULL || !(he->init & HASH_VALUE_SET)) {
		c = full_init_chunk(cnks, p);
		hash_set(cnks->h, (void*)p, c);
	} else {
		c = he->val;
	}

	return cnks->mem.e + c;
}

static void set_chunk_trav(struct chunk *a)
{
	int x, y;

	a->trav = 0;

	y = 0;
	for (x = 0; x < CHUNK_SIZE; x++)
		if (a->tiles[x][y] <= tile_forest) {
			a->trav |= trav_n;
			break;
		}

	y = CHUNK_SIZE - 1;
	for (x = 0; x < CHUNK_SIZE; x++)
		if (a->tiles[x][y] <= tile_forest) {
			a->trav |= trav_s;
			break;
		}

	x = 0;
	for (y = 0; y < CHUNK_SIZE; y++)
		if (a->tiles[x][y] <= tile_forest) {
			a->trav |= trav_w;
			break;
		}


	x = CHUNK_SIZE - 1;
	for (y = 0; y < CHUNK_SIZE; y++)
		if (a->tiles[x][y] <= tile_forest) {
			a->trav |= trav_e;
			break;
		}
}

static void fill_chunk(struct chunk *a)
{
	int x, y;
	float fx, fy, fcs = (float)CHUNK_SIZE;
	int noise;

//	L("generating chunk @ %d, %d", a->pos.x, a->pos.y);
	for (y = 0; y < CHUNK_SIZE; y++) {
		for (x = 0; x < CHUNK_SIZE; x++) {
			fx = (float)(x + a->pos.x) / (fcs * 2.0);
			fy = (float)(y + a->pos.y) / (fcs * 1.0);

			noise = (int)roundf(perlin_two(fx, fy, TPARAM_AMP, TPARAM_OCTS, TPARAM_FREQ, TPARAM_LACU)) + TPARAM_BOOST;

			a->tiles[x][y] = noise < 0 ? 0 : (noise > TILE_MAX ? TILE_MAX : noise);
		}
	}

	a->empty = 0;
	set_chunk_trav(a);
}

const struct chunk *get_chunk(struct chunks *cnks, const struct point *p)
{
	const struct chunk *c = get_chunk_no_gen(cnks, p);

	if (c->empty)
		fill_chunk((struct chunk *)c);

	return c;
}
