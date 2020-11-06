#include "posix.h"

#include <stdlib.h>
#include <string.h>

#ifdef CRTS_PATHFINDING
#include "shared/pathfind/api.h"
#include "shared/pathfind/preprocess.h"
#endif
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

void
chunks_init(struct chunks *cnks)
{
	memset(cnks, 0, sizeof(struct chunks));

	cnks->hd = hdarr_init(4096, sizeof(struct point), sizeof(struct chunk), NULL);

#ifdef CRTS_PATHFINDING
	abstract_graph_init(&cnks->ag);
#endif

#ifdef CRTS_SERVER
	cnks->storehouses = darr_init(sizeof(struct storehouse_storage));
	cnks->functional_tiles = hash_init(256, 1, sizeof(struct point));
	cnks->functional_tiles_buf = hash_init(256, 1, sizeof(struct point));
#endif
}

void
chunks_destroy(struct chunks *cnks)
{
	hdarr_destroy(cnks->hd);

#ifdef CRTS_PATHFINDING
	abstract_graph_destroy(&cnks->ag);
#endif

#ifdef CRTS_SERVER
	darr_destroy(cnks->storehouses);
	hash_destroy(cnks->functional_tiles);
	hash_destroy(cnks->functional_tiles_buf);
#endif
}

static struct chunk *
full_init_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk c = { 0 }, *cp = &c;

	uint32_t i;
	for (i = 0; i < CHUNK_SIZE * CHUNK_SIZE; ++i) {
		((float *)c.heights)[i] = -5;
	}

	c.pos = *p;
#ifdef CRTS_SERVER
	c.last_touched = cnks->chunk_date;
#endif

	hdarr_set(cnks->hd, p, cp);

	cp = hdarr_get(cnks->hd, p);

	return hdarr_get(cnks->hd, p);
}

static struct chunk *
get_chunk_no_gen(struct chunks *cnks, const struct point *p)
{
	const struct chunk *cnk;

	if ((cnk = hdarr_get(cnks->hd, p)) == NULL) {
		cnk = full_init_chunk(cnks, p);
	}

	assert(cnk->pos.x == p->x && cnk->pos.y == p->y);

	return (struct chunk *)cnk;
}

struct point
nearest_chunk(const struct point *p)
{
	return point_mod(p, CHUNK_SIZE);
}

struct chunk *
get_chunk(struct chunks *cnks, const struct point *p)
{
	struct chunk *c = get_chunk_no_gen(cnks, p);

	assert(!(p->x % CHUNK_SIZE) && !(p->y % CHUNK_SIZE));

	return c;
}

struct chunk *
get_chunk_at(struct chunks *cnks, const struct point *p)
{
	struct point np = nearest_chunk(p);

	return get_chunk(cnks, &np);
}

#ifdef CRTS_SERVER
void
touch_chunk(struct chunks *cnks, struct chunk *ck)
{
	ck->last_touched = ++cnks->chunk_date;
	ck->touched_this_tick |= true;
}
#endif

void
set_chunk(struct chunks *cnks, struct chunk *ck)
{
	/* L("setting chunk %d, %d", ck->pos.x, ck->pos.y); */
	hdarr_set(cnks->hd, &ck->pos, ck);
#ifdef CRTS_PATHFINDING
	ag_preprocess_chunk(cnks, ck);
#endif
}
