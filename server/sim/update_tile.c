#include "posix.h"

#include <assert.h>

#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"
#include "shared/pathfind/api.h"
#include "shared/util/log.h"

static void
commit_tile(struct world *w, const struct point *p, enum tile t)
{
	struct chunk *ck = get_chunk_at(&w->chunks, p);
	struct point rp = point_sub(p, &ck->pos);
	enum tile old_t = ck->tiles[rp.x][rp.y];

	assert(rp.x >= 0 && rp.x < CHUNK_SIZE);
	assert(rp.y >= 0 && rp.y < CHUNK_SIZE);

	if (t == old_t) {
		return;
	}

	if (gcfg.tiles[old_t].function) {
		hash_unset(&w->chunks.functional_tiles, p);
	}

	ck->tiles[rp.x][rp.y] = t;

	touch_chunk(&w->chunks, ck);

	if (gcfg.tiles[old_t].trav_type != gcfg.tiles[t].trav_type) {
		hpa_dirty_point(&w->chunks, p);
	}
}

float
update_tile_height(struct world *w, const struct point *p, float height)
{
	struct chunk *ck = get_chunk_at(&w->chunks, p);
	struct point rp = point_sub(p, &ck->pos);

	assert(rp.x >= 0 && rp.x < CHUNK_SIZE);
	assert(rp.y >= 0 && rp.y < CHUNK_SIZE);

	ck->heights[rp.x][rp.y] += height;

	touch_chunk(&w->chunks, ck);

	return ck->heights[rp.x][rp.y];
}

uint16_t
update_tile_ent_height(struct world *w, const struct point *p, int16_t delta)
{
	struct chunk *ck = get_chunk_at(&w->chunks, p);
	struct point rp = point_sub(p, &ck->pos);

	uint16_t *v = &ck->ent_height[rp.x][rp.y];

	assert(delta > 0 || (delta < 0 && *v));

	*v += delta;

	return *v;
}

void
update_tile(struct world *w, const struct point *p, enum tile t)
{
	assert(!gcfg.tiles[t].function);

	commit_tile(w, p, t);
}

void
update_functional_tile(struct world *w, const struct point *p, enum tile t,
	uint16_t mot, uint32_t age)
{
	union functional_tile ft;
	assert(gcfg.tiles[t].function);

	commit_tile(w, p, t);

	if (gcfg.tiles[t].function) {
		ft = (union functional_tile){ .ft = { .type = t,
						      .motivator = mot,
						      .age = age } };

		hash_set(&w->chunks.functional_tiles, p, ft.val);
	}
}
