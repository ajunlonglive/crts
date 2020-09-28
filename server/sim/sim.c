#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "server/sim/action.h"
#include "server/sim/ent.h"
#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/storehouse.h"
#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"
#include "shared/math/rand.h"
#include "shared/sim/tiles.h"
#include "shared/util/log.h"

static struct point
get_valid_spawn(struct chunks *chunks, uint8_t et)
{
	struct point p = {
		rand_uniform(gcfg.misc.initial_spawn_range),
		rand_uniform(gcfg.misc.initial_spawn_range)
	}, q;
	const struct chunk *ck;
	int i, j;

	p = nearest_chunk(&p);

	while (1) {
		p.x += CHUNK_SIZE;
		p.y += CHUNK_SIZE;
		ck = get_chunk(chunks, &p);

		for (i = 0; i < CHUNK_SIZE; i++) {
			for (j = 0; j < CHUNK_SIZE; j++) {
				if (tile_is_traversable(ck->tiles[i][j], et)) {
					q.x = i; q.y = j;
					return point_add(&p, &q);
				}
			}
		}
	}
}

static void
populate(struct simulation *sim, uint16_t amnt, uint16_t algn)
{
	size_t i;
	struct ent *e;
	struct point p = get_valid_spawn(&sim->world->chunks,
		gcfg.ents[et_worker].trav);

	for (i = 0; i < amnt; i++) {
		e = spawn_ent(sim->world);
		e->type = et_worker;
		e->pos = p;
		e->alignment = algn;
	}

	p = get_valid_spawn(&sim->world->chunks,
		gcfg.ents[et_vehicle_boat].trav);

	for (i = 0; i < 20; i++) {
		e = spawn_ent(sim->world);
		e->type = et_vehicle_boat;
		e->pos = p;
		e->alignment = algn;
	}
}

void
add_new_motivator(struct simulation *sim, uint16_t mot)
{
	populate(sim, gcfg.misc.initial_spawn_amount, mot);
}

struct simulation *
sim_init(struct world *w)
{
	struct simulation *sim = calloc(1, sizeof(struct simulation));

	ent_buckets_init(&sim->eb);
	sim->world = w;
	sim_actions_init(sim);

	return sim;
}

static enum iteration_result
process_graveyard_iterator(void *_s, void *_id)
{
	uint16_t *id = _id;
	struct simulation *s = _s;

	world_despawn(s->world, *id);

	return ir_cont;
}

void
harvest_tile(struct world *w, struct point *p, uint16_t mot, uint32_t tick)
{
	struct ent *drop;
	enum tile t = get_tile_at(&w->chunks, p);

	if (gcfg.tiles[t].drop) {
		drop = spawn_ent(w);
		drop->pos = *p;
		drop->type = gcfg.tiles[t].drop;
	}

	if (gcfg.tiles[gcfg.tiles[t].base].function) {
		update_functional_tile(w, p, gcfg.tiles[t].base, mot, tick);
	} else {
		update_tile(w, p, gcfg.tiles[t].base);
	}
}
void
simulate(struct simulation *sim)
{
	darr_clear_iter(sim->world->graveyard, sim, process_graveyard_iterator);
	darr_clear_iter(sim->world->spawn, sim, process_spawn_iterator);
	actions_flush(sim);

	process_environment(sim);

	ent_buckets_clear(&sim->eb);
	make_ent_buckets(sim->world->ents, &sim->eb);

	hdarr_for_each(sim->actions, sim, action_process);
	hdarr_for_each(sim->world->ents, sim, simulate_ent);

	if (sim->tick & 0xff) {
		process_storehouses(sim->world);
	}

	++sim->tick;
}
