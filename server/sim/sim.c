#include "posix.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "server/sim/ent.h"
#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"
#include "shared/math/rand.h"
#include "shared/serialize/limits.h"
#include "shared/sim/tiles.h"
#include "shared/util/log.h"
#include "tracy.h"

static struct point
get_valid_spawn(struct chunks *chunks, uint8_t et)
{
	struct point p = {
		rand_uniform(gcfg.misc.initial_spawn_range),
		rand_uniform(gcfg.misc.initial_spawn_range)
	}, q;
	const struct chunk *ck;
	int i, j;

	while (1) {
		uint32_t cx = rand_uniform(chunks->w);
		uint32_t cy = rand_uniform(chunks->h);

		p = (struct point) { .x = cx * CHUNK_SIZE, .y = cy * CHUNK_SIZE };
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
populate(struct simulation *sim, uint16_t amnt, uint16_t algn,
	const struct point *spawn)
{
	size_t i;
	struct ent *e;

	for (i = 0; i < amnt; i++) {
		e = spawn_ent(sim->world);
		e->type = et_worker;
		e->pos = *spawn;
		e->alignment = algn;

		e->age = rand_uniform(gcfg.ents[et_worker].lifespan / 2);
	}
}

struct player *
add_new_player(struct simulation *sim, uint16_t id)
{
	uint32_t idx = darr_push(&sim->players, &(struct player){ .id = id });

	struct point spawn = get_valid_spawn(&sim->world->chunks, trav_land);

	populate(sim, gcfg.misc.initial_spawn_amount, id, &spawn);

	struct player *p = darr_get(&sim->players, idx);

	p->cursor = spawn;

	return p;
}

struct player *
get_player(struct simulation *sim, uint16_t id)
{
	uint32_t i;
	struct player *p;
	for (i = 0; i < sim->players.len; ++i) {
		p = darr_get(&sim->players, i);
		if (p->id == id) {
			return p;
		}
	}

	return NULL;
}

struct player *
get_nearest_player(struct simulation *sim, struct point *pos, uint32_t max)
{
	uint32_t i, min = UINT32_MAX, dist;
	struct player *p, *minp = NULL;

	for (i = 0; i < sim->players.len; ++i) {
		p = darr_get(&sim->players, i);
		if ((dist = square_dist(pos, &p->cursor) < max && dist < min)) {
			minp = p;
			min = dist;
		}
	}

	return minp;
}

void
sim_init(struct world *w, struct simulation *sim)
{
	ent_buckets_init(&sim->eb);
	sim->world = w;
	darr_init(&sim->players, sizeof(struct player));
	hdarr_init(&sim->terrain_mods, 2048, sizeof(struct point), sizeof(struct terrain_mod), NULL);
}

static enum iteration_result
process_graveyard_iterator(void *_s, void *_id)
{
	uint16_t *id = _id;
	struct simulation *s = _s;

	world_despawn(s->world, *id);

	return ir_cont;
}

static void
reset_player_counted_stats(struct simulation *sim)
{
	uint32_t i;
	for (i = 0; i < sim->players.len; ++i) {
		struct player *p = darr_get(&sim->players, i);
		p->ent_count = 0;
		p->ent_center_of_mass = (struct point){ 0 };
	}
}

static void
update_player_counted_stats(struct simulation *sim)
{
	uint32_t i;
	for (i = 0; i < sim->players.len; ++i) {
		struct player *p = darr_get(&sim->players, i);
		if (p->ent_count) {
			p->ent_center_of_mass.x /= p->ent_count;
			p->ent_center_of_mass.y /= p->ent_count;
		}
	}
}

static void
modify_terrain(struct simulation *sim)
{
	TracyCZoneAutoS;
	static const uint16_t r = 5;
	const float rs = r * r;
	struct terrain_mod *tm;
	int16_t x, y;
	float sdist, tmpdh;
	struct point q;
	struct point cp;
	struct chunk *ck;
	uint32_t i;

	for (i = 0; i < sim->terrain_mods.darr.len; ++i) {
		tm = hdarr_get_by_i(&sim->terrain_mods, i);

		ck = NULL;

		for (x = -r; x < r; ++x) {
			for (y = -r; y < r; ++y) {
				if ((sdist = (x * x + y * y)) <= rs) {
					tmpdh = tm->mod * (1.0f - (sdist / rs));
					q = (struct point) { tm->pos.x + x, tm->pos.y + y };
					cp = nearest_chunk(&q);
					if (!ck || !points_equal(&cp, &ck->pos)) {
						ck = get_chunk(&sim->world->chunks, &cp);
						touch_chunk(&sim->world->chunks, ck);
					}
					cp = point_sub(&q, &cp);
					if (tmpdh < 0.0f && ck->heights[cp.x][cp.y] < 0.1f) {
						continue;
					}
					ck->heights[cp.x][cp.y] += tmpdh;

					if (ck->heights[cp.x][cp.y] > MAX_HEIGHT) {
						ck->heights[cp.x][cp.y] = MAX_HEIGHT;
					} else if (ck->tiles[cp.x][cp.y] == tile_sea) {
						if (ck->heights[cp.x][cp.y] > 0.0f) {
							ck->tiles[cp.x][cp.y] = tile_coast;
						}
					}
				}
			}
		}

	}
	TracyCZoneAutoE;
}

void
simulate(struct simulation *sim)
{
	TracyCZoneAutoS;

	TracyCZoneN(tctx_graveyard, "graveyard", true);
	darr_clear_iter(&sim->world->graveyard, sim, process_graveyard_iterator);
	TracyCZoneEnd(tctx_graveyard);

	TracyCZoneN(tctx_spawn, "spawn", true);
	darr_clear_iter(&sim->world->spawn, sim, process_spawn_iterator);
	TracyCZoneEnd(tctx_spawn);

	/* process_environment(sim); */

	reset_player_counted_stats(sim);

	make_ent_buckets(&sim->eb, &sim->world->ents);

	simulate_ents(sim);
	modify_terrain(sim);
	hdarr_clear(&sim->terrain_mods);

	update_player_counted_stats(sim);

	++sim->tick;

	hpa_clean(&sim->world->chunks);

	TracyCZoneAutoE;
}
