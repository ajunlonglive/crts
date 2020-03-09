#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"

#define GROW_CHANCE 100 // 1 in

static void
process_chunk(struct chunks *cnks, struct chunk *ck)
{
	size_t x, y;
	enum tile t;

	for (x = 0; x < CHUNK_SIZE; ++x) {
		for (y = 0; y < CHUNK_SIZE; ++y) {
			if (random() % GROW_CHANCE != 0) {
				continue;
			}

			t = ck->tiles[x][y];

			update_tile_at(cnks, ck, x, y, t);
		}
	}
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

	process_chunk(sim->world->chunks, ck);
}
