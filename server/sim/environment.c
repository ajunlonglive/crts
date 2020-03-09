#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "shared/constants/globals.h"

#define GROW_CHANCE 100 // 1 in

static bool
process_chunk(struct chunk *ck)
{
	size_t x, y;
	enum tile t;
	bool changed = false;

	for (x = 0; x < CHUNK_SIZE; ++x) {
		for (y = 0; y < CHUNK_SIZE; ++y) {
			if (random() % GROW_CHANCE != 0) {
				continue;
			}

			t = ck->tiles[x][y];

			ck->tiles[x][y] = gcfg.tile_lifecycle[t].next;
			changed = true;
		}
	}

	return changed;
}

void
process_environment(struct simulation *sim)
{
	size_t ri;

	if ((ri = hdarr_len(sim->world->chunks->hd)) == 0) {
		return;
	}

	ri = random() % ri;

	struct chunk *ck = hdarr_get_by_i(sim->world->chunks->hd, ri);

	if (process_chunk(ck)) {
		ck->last_touched = ++sim->world->chunks->chunk_date;
	}
}
