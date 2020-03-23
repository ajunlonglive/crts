#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include <stdlib.h>

#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/terrain.h"
#include "shared/constants/globals.h"
#include "shared/sim/alignment.h"
#include "shared/util/log.h"

#define SHRINE_SPAWN_RATE 64

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
			alignment_adjust(e->alignment, ft.ft.motivator, 9999);
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
