#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdlib.h>

#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"

static uint32_t
determine_grow_chance(struct chunks *cnks, int x, int y, enum tile t)
{
	uint8_t adj = 0;
	size_t i;
	struct chunk *ck;
	struct point np, rp, p[4] = {
		{ x + 1, x     },
		{ x - 1, x     },
		{ x,     x + 1 },
		{ x,     x - 1 },
	};

	for (i = 0; i < 4; ++i) {
		np = nearest_chunk(&p[i]);
		if ((ck = hdarr_get(cnks->hd, &np)) != NULL) {
			rp = point_sub(&np, &p[i]);
			if (t == ck->tiles[rp.x][rp.y]) {
				++adj;
			}
		}
	}

	return adj > 0 ? 2000 / adj : 0;
}

static enum iteration_result
process_chunk(struct chunks *cnks, struct chunk *ck)
{
	enum tile t, nt;
	size_t x, y;
	struct point p = ck->pos;
	uint32_t chance;

	for (x = 0; x < CHUNK_SIZE; ++x) {
		for (y = 0; y < CHUNK_SIZE; ++y) {
			ck = hdarr_get(cnks->hd, &p);
			t = ck->tiles[x][y];
			nt = gcfg.tiles[t].next;

			chance = determine_grow_chance(cnks,
				x + ck->pos.x, y + ck->pos.y, t);

			if (chance <= 0 || random() % chance != 0) {
				continue;
			}

			update_tile_at(cnks, ck, x, y, nt);
		}
	}

	return ir_cont;
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
	//hdarr_for_each(sim->world->chunks->hd, sim->world->chunks, process_chunk);
}
