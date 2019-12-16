#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <math.h>

#include "math/perlin.h"
#include "util/log.h"
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

static struct chunk *full_init_chunk(const struct point *p)
{
	struct chunk *c = NULL;

	chunk_init(&c);
	c->pos = *p;

	return c;
}

static const struct chunk *get_chunk_no_gen(struct hash *chunks, const struct point *p)
{
	const struct chunk *c;

	if ((c = hash_get(chunks, (void*)p)) == NULL) {
		c = full_init_chunk(p);
		hash_set(chunks, (void*)p, c);
	}

	return c;
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
	L("got chunk trav: %0x", a->trav);
}

static void fill_chunk(struct chunk *a)
{
	int x, y;
	float fx, fy, fcs = (float)CHUNK_SIZE;
	int noise;

	L("generating chunk @ %d, %d", a->pos.x, a->pos.y);
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

const struct chunk *get_chunk(struct hash *chunks, struct point *p)
{
	const struct chunk *c = get_chunk_no_gen(chunks, p);

	if (c->empty)
		fill_chunk((struct chunk *)c);

	return c;
}
