#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdlib.h>

#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"

#define SHRINE_SPAWN_RATE 64

static enum iteration_result
process_chunk(struct chunks *cnks, struct chunk *ck)
{
	if (age_chunk(ck)) {
		touch_chunk(cnks, ck);
	}

	return ir_cont;
}

static void
process_random_chunk(struct simulation *sim)
{
	size_t ri;

	if ((ri = hdarr_len(sim->world->chunks->hd)) == 0) {
		return;
	}

	ri = random() % ri;

	struct chunk *ck = hdarr_get_by_i(sim->world->chunks->hd, ri);

	process_chunk(sim->world->chunks, ck);
}

static enum iteration_result
process_functional_tiles(void *_sim, void *_p, size_t val)
{
	struct point q, *p = _p;
	struct simulation *sim = _sim;
	struct ent *e;

	union functional_tile ft = { .val = val };

	switch (ft.ft.type) {
	case tile_shrine:
		if (sim->tick % SHRINE_SPAWN_RATE == 0) {
			if (!find_adj_tile(sim->world->chunks, p, &q, NULL, -1,
				tile_is_traversable)) {
				L("no valid places to spawn");
				return ir_cont;
			}

			e = spawn_ent(sim);
			e->pos = q;
			e->alignment = ft.ft.motivator;
			e->type = et_worker;
		}
		break;
	default:
		break;
	}

	return ir_cont;
}

void
process_environment(struct simulation *sim)
{
	process_random_chunk(sim);

	//hdarr_for_each(sim->world->chunks->hd, sim->world->chunks, process_chunk);

	hash_for_each_with_keys(sim->world->chunks->functional_tiles, sim,
		process_functional_tiles);
}
