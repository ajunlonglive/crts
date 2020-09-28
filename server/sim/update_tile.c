#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <assert.h>

#include "server/sim/storehouse.h"
#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"

static void
commit_tile(struct world *w, const struct point *p, enum tile t)
{
	struct chunk *ck = get_chunk_at(&w->chunks, p);
	struct point rp = point_sub(p, &ck->pos);

	assert(rp.x >= 0 && rp.x < CHUNK_SIZE);
	assert(rp.y >= 0 && rp.y < CHUNK_SIZE);

	if (t == ck->tiles[rp.x][rp.y]) {
		return;
	}

	/* TODO: this is a hack, make a general system for height modification */
	if (ck->tiles[rp.x][rp.y] == tile_mountain) {
		ck->heights[rp.x][rp.y] -= 2.0;
	}

	switch (gcfg.tiles[ck->tiles[rp.x][rp.y]].function) {
	case tfunc_dynamic:
		hash_unset(w->chunks.functional_tiles, p);
		break;
	case tfunc_storage:
		destroy_storehouse(w, p);
		break;
	default:
		break;
	}

	ck->tiles[rp.x][rp.y] = t;
	ck->harvested[rp.x][rp.y] = 0;

	touch_chunk(&w->chunks, ck);
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

	switch (gcfg.tiles[t].function) {
	case tfunc_dynamic:
		ft = (union functional_tile){ .ft = { .type = t,
						      .motivator = mot,
						      .age = age } };

		hash_set(w->chunks.functional_tiles, p, ft.val);
		break;
	case tfunc_storage:
		create_storehouse(w, p, mot);
		break;
	default:
		assert(false);
		break;
	}
}
