#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "shared/pathfind/api.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

void
chunks_init(struct chunks *cnks)
{
	memset(cnks, 0, sizeof(struct chunks));

	hdarr_init(&cnks->hd, 16384, sizeof(struct point), sizeof(struct chunk), NULL);

	abstract_graph_init(&cnks->ag);

	hash_init(&cnks->functional_tiles, 256, sizeof(struct point));
	hash_init(&cnks->functional_tiles_buf, 256, sizeof(struct point));
}

void
chunks_destroy(struct chunks *cnks)
{
	hdarr_destroy(&cnks->hd);

	abstract_graph_destroy(&cnks->ag);

	hash_destroy(&cnks->functional_tiles);
	hash_destroy(&cnks->functional_tiles_buf);
}

static struct chunk *
full_init_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk c = { 0 }, *cp = &c;

	uint32_t h = p->y / CHUNK_SIZE, w = p->x / CHUNK_SIZE;

	if (h > cnks->h) {
		cnks->h = h;
	}

	if (w > cnks->w) {
		cnks->w = w;
	}

	uint32_t i;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		((float *)c.heights)[i] = -5;
	}

	c.pos = *p;
	c.last_touched = cnks->chunk_date;

	hdarr_set(&cnks->hd, p, cp);

	cp = hdarr_get(&cnks->hd, p);

	return hdarr_get(&cnks->hd, p);
}

struct point
nearest_chunk(const struct point *p)
{
	return point_mod(p, CHUNK_SIZE);
}

struct chunk *
get_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk *c;

	assert(!(p->x % CHUNK_SIZE) && !(p->y % CHUNK_SIZE));

	if (!(c = hdarr_get(&cnks->hd, p))) {
		c = full_init_chunk(cnks, p);
	}

	return c;
}

struct chunk *
get_chunk_at(struct chunks *cnks, const struct point *p)
{
	struct point np = nearest_chunk(p);

	return get_chunk(cnks, &np);
}

void
touch_chunk(struct chunks *cnks, struct chunk *ck)
{
	ck->last_touched = ++cnks->chunk_date;
	ck->touched_this_tick |= true;
}

void
set_chunk(struct chunks *cnks, struct chunk *ck)
{
	/* L(log_misc, "setting chunk %d, %d", ck->pos.x, ck->pos.y); */
	hdarr_set(&cnks->hd, &ck->pos, ck);
}
