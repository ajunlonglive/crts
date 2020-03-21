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
determine_grow_chance(struct chunks *cnks, struct point *c, enum tile t)
{
	uint8_t adj = 0;
	size_t i;
	struct chunk *ck;
	struct point np, rp, p[4] = {
		{ c->x + 1, c->y     },
		{ c->x - 1, c->y     },
		{ c->x,     c->y + 1 },
		{ c->x,     c->y - 1 },
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
	struct point p = ck->pos, c, d;
	uint32_t chance;

	for (c.x = 0; c.x < CHUNK_SIZE; ++c.x) {
		for (c.y = 0; c.y < CHUNK_SIZE; ++c.y) {
			ck = hdarr_get(cnks->hd, &p);
			t = ck->tiles[c.x][c.y];

			if (!(nt = gcfg.tiles[t].next)) {
				continue;
			}

			d = point_add(&c, &ck->pos);
			chance = determine_grow_chance(cnks, &d, t);

			if (chance <= 0 || random() % chance != 0) {
				continue;
			}

			update_tile(cnks, &d, nt);
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
