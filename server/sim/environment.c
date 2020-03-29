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
	enum ent_type et = gcfg.misc.spawnable_ents[random() % SPAWNABLE_ENTS_LEN];

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
burn_spread(struct chunks *cnks, struct point *p, uint32_t tick)
{
	size_t i;
	struct point c[4] = {
		{ p->x + 1, p->y     },
		{ p->x - 1, p->y     },
		{ p->x,     p->y + 1 },
		{ p->x,     p->y - 1 },
	};

	for (i = 0; i < 4; ++i) {
		if (gcfg.tiles[get_tile_at(cnks, &c[i])].flamable) {
			if (!(random() % gcfg.misc.fire_spread_ignite_chance)) {
				update_functional_tile(cnks, &c[i], tile_burning, 0, tick);
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

struct find_food_ctx {
	struct circle *range;
};

static bool
find_food_pred(void *_ctx, struct ent *e)
{
	struct find_food_ctx *ctx = _ctx;

	return (e->type == et_resource_meat || e->type == et_resource_crop)
	       && (ctx->range ? point_in_circle(&e->pos, ctx->range) : true);
}

struct ent *
find_food(struct world *w, struct point *p, struct circle *c)
{
	struct find_food_ctx ctx = { c };

	return find_ent(w, p, &ctx, find_food_pred);
}

static enum iteration_result
process_functional_tiles(void *_sim, void *_p, size_t val)
{
	struct point q, *p = _p;
	struct circle c;
	struct simulation *sim = _sim;
	struct ent *e;
	int32_t tmp;

	union functional_tile ft = { .val = val };

	switch (ft.ft.type) {
	case tile_shrine:
		if (sim->tick % gcfg.misc.shrine_spawn_rate == 0) {
			c.center = *p;
			c.r = gcfg.misc.shrine_range;

			if (!find_adj_tile(sim->world->chunks, p, &q, NULL, -1,
				tile_is_traversable)) {
				L("no valid places to spawn");
				return ir_cont;
			} else if ((e = find_food(sim->world, p, &c)) == NULL) {
				return ir_cont;
			}

			kill_ent(sim, e);

			e = spawn_ent(sim);
			e->pos = q;
			e->alignment = ft.ft.motivator;
			e->type = et_worker;
		}
		break;
	case tile_farmland_empty:
		tmp = sim->tick - ft.ft.tick;
		tmp = tmp < 0 ? 0 - tmp : tmp;

		if (tmp > gcfg.misc.farm_grow_rate) {
			update_tile(sim->world->chunks, p, tile_farmland_done);
		}
		break;
	case tile_burning:
		tmp = sim->tick - ft.ft.tick;
		tmp = tmp < 0 ? 0 - tmp : tmp;

		if (tmp > gcfg.misc.fire_spread_rate &&
		    !(random() % gcfg.misc.fire_spread_chance)) {
			burn_spread(sim->world->chunks, p, sim->tick);
			update_tile(sim->world->chunks, p, tile_burnt);
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
