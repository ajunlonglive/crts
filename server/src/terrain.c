#define _XOPEN_SOURCE 500
#include <stdlib.h>

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
	struct chunk *c;

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
	struct point p;
	int x, y;

	for (x = 0; x < CHUNK_SIZE; x++) {
		for (y = 0; y < CHUNK_SIZE; y++) {
			p.x = x + a->pos.x;
			p.y = y + a->pos.y;

			a->tiles[x][y] =
				perlin_two((float)p.x / (float)CHUNK_SIZE, (float)p.y / (float)CHUNK_SIZE, 3, 2, 1);
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
