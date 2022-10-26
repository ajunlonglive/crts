#include "posix.h"

#include <stdlib.h>

#include "server/sim/ent.h"
#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"
#include "shared/math/rand.h"
#include "shared/sim/tiles.h"
#include "shared/util/log.h"
#include "tracy.h"

uint32_t
determine_grow_chance(struct chunk *ck, int32_t x, int32_t y, enum tile t)
{
	struct point p[4] = {
		{ x + 1, y     },
		{ x - 1, y     },
		{ x,     y + 1 },
		{ x,     y - 1 },
	};
	uint8_t adj = 0;
	size_t i;

	enum tile trigger = gcfg.tiles[t].next_to;
	if (!trigger) {
		trigger = t;
	}

	for (i = 0; i < 4; ++i) {
		if (p[i].x < 0 || p[i].x >= CHUNK_SIZE || p[i].y < 0 || p[i].y >= CHUNK_SIZE) {
			continue;
		}

		if (trigger == ck->tiles[p[i].x][p[i].y]) {
			++adj;
		}
	}

	return adj;
}

static bool
age_chunk(struct chunk *ck)
{
	enum tile t, nt;
	struct point c;
	uint32_t chance;
	bool updated = false;

	for (c.x = 0; c.x < CHUNK_SIZE; ++c.x) {
		for (c.y = 0; c.y < CHUNK_SIZE; ++c.y) {
			t = ck->tiles[c.x][c.y];

			if (!(nt = gcfg.tiles[t].next)) {
				continue;
			}

			chance = determine_grow_chance(ck, c.x, c.y, t);

			if (chance == 1) {
				ck->tiles[c.x][c.y] = gcfg.tiles[t].next;
				updated = true;
			}
		}
	}

	return updated;
}

static void
spawn_random_creature(struct simulation *sim, struct chunk *ck)
{
	struct point c;
	int i, amnt;
	enum ent_type et = gcfg.misc.spawnable_ents[rand_uniform(SPAWNABLE_ENTS_LEN)];

	for (c.x = 0; c.x < CHUNK_SIZE; ++c.x) {
		for (c.y = 0; c.y < CHUNK_SIZE; ++c.y) {
			if (ck->tiles[c.x][c.y] == gcfg.ents[et].spawn_tile) {
				if (rand_chance(gcfg.ents[et].spawn_chance)) {
					amnt = gcfg.ents[et].group_size;

					for (i = 0; i < amnt; ++i) {
						struct point p = point_add(&c, &ck->pos);
						spawn_ent(sim->world, et, &p);
					}
				}
			}
		}
	}
}

static enum iteration_result
process_chunk(void *_sim, void *_ck)
{
	struct simulation *sim = _sim;
	struct chunk *ck = _ck;

	if (rand_chance(10000)) {
		spawn_random_creature(sim, ck);
	}

	if (age_chunk(ck)) {
		touch_chunk(&sim->world->chunks, ck);
	}

	return ir_cont;
}

static void
burn_spread(struct world *w, struct point *p)
{
	size_t i;
	struct point c[4] = {
		{ p->x + 1, p->y     },
		{ p->x - 1, p->y     },
		{ p->x,     p->y + 1 },
		{ p->x,     p->y - 1 },
	};

	for (i = 0; i < 4; ++i) {
		if (gcfg.tiles[get_tile_at(&w->chunks, &c[i])].flamable) {
			if (rand_chance(gcfg.misc.fire_spread_ignite_chance)) {
				update_functional_tile(w, &c[i], tile_fire, 0, 0);
			}
		}

	}
}

static enum iteration_result
process_functional_tiles(void *_sim, void *_p, uint64_t val)
{
	struct point *p = _p;
	struct simulation *sim = _sim;

	union functional_tile ft = { .val = val };

	switch (ft.ft.type) {
	case tile_fire:
		if (ft.ft.age > gcfg.misc.fire_spread_rate &&
		    rand_chance(gcfg.misc.fire_spread_chance)) {
			burn_spread(sim->world, p);
			update_tile(sim->world, p, tile_ash);
		} else {
			update_functional_tile(sim->world, p,
				tile_fire, 0, ft.ft.age + 1);
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
	TracyCZoneAutoS;

	if (!(sim->tick & 7)) {
		TracyCZoneN(tctx_process_chunks, "process chunks", true);
		hdarr_for_each(&sim->world->chunks.hd, sim, process_chunk);
		TracyCZoneEnd(tctx_process_chunks);
	}

	struct hash tmp = sim->world->chunks.functional_tiles;

	sim->world->chunks.functional_tiles = sim->world->chunks.functional_tiles_buf;

	hash_for_each_with_keys(&tmp, sim, process_functional_tiles);

	hash_clear(&tmp);
	sim->world->chunks.functional_tiles_buf = tmp;
	TracyCZoneAutoE;
}
