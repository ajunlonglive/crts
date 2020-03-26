#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdlib.h>

#include "server/sim/do_action.h"
#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/util/log.h"

#define SHRINE_SPAWN_RATE 64
#define SHRINE_SACRIFICE_RANGE 5
#define SHRINE_SACRIFICE et_resource_meat

#define RANDOM_ENTS 1
static const enum ent_type random_ents[RANDOM_ENTS] = {
	et_deer
};

static enum iteration_result
process_chunk(struct chunks *cnks, struct chunk *ck)
{
	if (age_chunk(ck)) {
		touch_chunk(cnks, ck);
	}

	return ir_cont;
}

static void
spawn_random_creature(struct simulation *sim, struct chunk *ck)
{
	struct point c;
	struct ent *spawn;
	int i, amnt;
	enum ent_type et = random_ents[random() % RANDOM_ENTS];

	for (c.x = 0; c.x < CHUNK_SIZE; ++c.x) {
		for (c.y = 0; c.y < CHUNK_SIZE; ++c.y) {
			if (ck->tiles[c.x][c.y] == gcfg.ents[et].spawn_tile) {
				if (random() % gcfg.ents[et].spawn_chance == 0) {
					amnt = gcfg.ents[et].group_size;

					for (i = 0; i < amnt; ++i) {
						spawn = spawn_ent(sim);
						spawn->type = et;
						spawn->pos = point_add(&c, &ck->pos);
					}
				}
			}
		}
	}
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

	spawn_random_creature(sim, ck);
}

static enum iteration_result
process_functional_tiles(void *_sim, void *_p, size_t val)
{
	struct point q, *p = _p;
	struct circle c;
	struct simulation *sim = _sim;
	struct ent *e;

	union functional_tile ft = { .val = val };

	switch (ft.ft.type) {
	case tile_shrine:
		if (sim->tick % SHRINE_SPAWN_RATE == 0) {
			c.center = *p;
			c.r = SHRINE_SACRIFICE_RANGE;

			if (!find_adj_tile(sim->world->chunks, p, &q, NULL, -1,
				tile_is_traversable)) {
				L("no valid places to spawn");
				return ir_cont;
			} else if ((e = find_resource(sim->world,
				SHRINE_SACRIFICE, p, &c)) == NULL) {
				return ir_cont;
			}

			kill_ent(sim, e);

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
