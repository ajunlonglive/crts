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

#define GRADIENT_MAX 64

static struct chunk *full_init_chunk(const struct point *p)
{
	struct chunk *c = NULL;

	chunk_init(&c);
	c->pos = *p;

	return c;
}

static struct chunk *get_chunk_no_gen(struct world *w, const struct point *p)
{
	struct chunk *c;

	if ((c = hash_get(w->chunks, (void*)p)) == NULL) {
		c = full_init_chunk(p);
		hash_set(w->chunks, (void*)p, c);
	}

	return c;
}

static void fill_chunk(struct world *w, struct chunk *a)
{
	int x, y;
	float fx, fy, fcs = (float)CHUNK_SIZE;

	L("generating chunk @ %d, %d", a->pos.x, a->pos.y);
	for (y = 0; y < CHUNK_SIZE; y++) {
		for (x = 0; x < CHUNK_SIZE; x++) {
			fx = (float)(x + a->pos.x) / (fcs * 2.0);
			fy = (float)(y + a->pos.y) / (fcs * 1.0);


			//L("x: %f, y: %f, n: %f", fx, fy, perlin_two(fx, fy, 2, 3, 2));
			a->tiles[x][y] = (int)roundf(perlin_two(fx, fy, 2.0, 3, 2.0)) + 3;
			//L("%d", a->tiles[x][y]);
		}
	}

	a->empty = 0;
}

struct chunk *get_chunk(struct world *w, struct point *p)
{
	struct chunk *c = get_chunk_no_gen(w, p);

	if (c->empty)
		fill_chunk(w, c);

	return c;
}
