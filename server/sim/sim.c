#include "posix.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "server/sim/ent.h"
#include "server/sim/environment.h"
#include "server/sim/sim.h"
#include "server/sim/update_tile.h"
#include "shared/constants/globals.h"
#include "shared/math/linalg.h"
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
	uint32_t j;
	struct point p = { 271, 246 };
	for (j = 0; j < 1; ++j) {
		struct ent *e = spawn_ent(sim->world, et_sand, &p);
		p.x += 1;
		p.y += 1;
		e->z = 32 + j;
	}
	return;

	int16_t r = 5;
	const float rs = r * r;
	int16_t x, y;
	float sdist;
	for (x = -r; x < r; ++x) {
		for (y = -r; y < r; ++y) {
			if ((sdist = (x * x + y * y)) <= rs) {
				sdist = r - sqrtf(sdist);
				for (j = 0; j < sdist; ++j) {
					struct point spawn_pos = p;
					spawn_pos.x += x;
					spawn_pos.y += y;
					spawn_ent(sim->world, et_sand, &spawn_pos);
				}
			}
		}
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
	uint32_t i, min = UINT32_MAX, dist = 0;
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
	sim->t = 1 / 60.0f;
	hash_init(&sim->eb, 2048, sizeof(struct point3d));
	sim->world = w;
	darr_init(&sim->players, sizeof(struct player));
	darr_init(&sim->terrain_mods, sizeof(struct terrain_mod));
	darr_init(&sim->ents_sorted, sizeof(struct ent *));
}

void
sim_reset(struct simulation *sim)
{
	darr_clear(&sim->players);
	darr_clear(&sim->terrain_mods);

	sim->seq = 0;
	sim->chunk_date = 0;
	sim->tick = 0;
	sim->paused = false;
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
handle_player_actions(struct simulation *sim)
{
	uint32_t i;
	for (i = 0; i < sim->players.len; ++i) {
		struct player *p = darr_get(&sim->players, i);

		switch (p->action) {
		case act_create: {
			uint16_t r = 2;

			if (p->action_arg == et_spring) {
				r = 1;
			} else if (p->action_arg == et_wood) {
				r = 1;
			}

			int16_t x, y;
			for (x = 0; x < r; ++x) {
				for (y = 0; y < r; ++y) {
					struct point spawn_pos = p->cursor;
					spawn_pos.x += x;
					spawn_pos.y += y;
					struct ent *e = spawn_ent(sim->world, p->action_arg, &spawn_pos);
					e->z = p->cursor_z;
				}
			}

			break;
		}
		case act_destroy: {
			struct point3d key = { p->cursor.x, p->cursor.y, p->cursor_z };

			uint64_t *ptr;
			if ((ptr = hash_get(&sim->eb, &key))) {
				struct ent *e = (struct ent *)*ptr;
				LOG_W(log_misc, "deleting (%d, %d, %d), %d", key.x, key.y, key.z, e->id);
				kill_ent(sim, e);
			}
			break;
		}
		case act_terrain: {
			switch ((enum act_terrain_arg)p->action_arg) {
			case act_terrain_raise:
				darr_push(&sim->terrain_mods, &(struct terrain_mod) {
						.r = 15,
						.pos = { p->cursor.x, p->cursor.y },
						.type = terrain_mod_height,
						.mod.height = 0.2f,
					});
				break;
			case act_terrain_lower:
				darr_push(&sim->terrain_mods, &(struct terrain_mod) {
						.r = 15,
						.pos = { p->cursor.x, p->cursor.y },
						.type = terrain_mod_height,
						.mod.height = -0.2f,
					});
				break;
			case act_terrain_flatten:
				darr_push(&sim->terrain_mods, &(struct terrain_mod) {
						.r = 15,
						.pos = { p->cursor.x, p->cursor.y },
						.type = terrain_mod_level,
						.mod.level = {
							.tgt = get_height_at(&sim->world->chunks, &p->cursor),
							.intensity = 0.01f,
						},
					});
				break;
			case act_terrain_arg_count:
				break;
			}
		}
		case act_neutral:
		case action_count:
			break;
		}

		if (p->do_action_once) {
			p->action = act_neutral;
		}
	}
}

static void
modify_terrain(struct simulation *sim)
{
	TracyCZoneAutoS;
	struct terrain_mod *tm;
	int16_t x, y, z;
	uint16_t r, rz;
	float tmpdh, rs;
	struct point q;
	struct point cp;
	struct chunk *ck;
	uint32_t i;

	for (i = 0; i < sim->terrain_mods.len; ++i) {
		tm = darr_get(&sim->terrain_mods, i);

		r = tm->r;
		rs = r * r;
		rz = 0;

		if (tm->type == terrain_mod_crater) {
			rz = r;
		}

		ck = NULL;

		for (z = -rz; z <= rz; ++z) {
			for (x = -r; x <= r; ++x) {
				for (y = -r; y <= r; ++y) {
					float dist3 = x * x + y * y + z * z;
					float unit_distance_from_center = dist3 / rs;
					if (unit_distance_from_center > 1.0f) {
						continue;
					}

					q = (struct point) { tm->pos.x + x, tm->pos.y + y };
					cp = nearest_chunk(&q);
					if (!ck || !points_equal(&cp, &ck->pos)) {
						ck = get_chunk(&sim->world->chunks, &cp);
					}
					cp = point_sub(&q, &cp);

					switch (tm->type) {
					case terrain_mod_crater: {
						float realz = tm->pos.z + z;

						/* terrain modifications below */
						if (!(realz <= ck->heights[cp.x][cp.y])) {
							continue;
						}

						float inv_dist = (1.0f - unit_distance_from_center);
						tmpdh = tm->mod.height * inv_dist + ((float)drand48() - 0.75f) * 0.1;
						touch_chunk(&sim->world->chunks, ck);
						ck->heights[cp.x][cp.y] += tmpdh;

						if (unit_distance_from_center < 0.66
						    && (ck->tiles[cp.x][cp.y]  == tile_tree || ck->tiles[cp.x][cp.y] == tile_old_tree) ) {
							ck->tiles[cp.x][cp.y] = tile_plain;

						}

						if (rand_chance(unit_distance_from_center * 50)) {
							ck->tiles[cp.x][cp.y] = tile_ash;
						}
						break;
					}
					case terrain_mod_level: {
						float diff = tm->mod.level.tgt - ck->heights[cp.x][cp.y];
						ck->heights[cp.x][cp.y] += diff * tm->mod.level.intensity;
						touch_chunk(&sim->world->chunks, ck);
						break;
					}
					case terrain_mod_height:
						tmpdh = tm->mod.height * (1.0f - unit_distance_from_center);
						touch_chunk(&sim->world->chunks, ck);
						ck->heights[cp.x][cp.y] += tmpdh;

						if (ck->heights[cp.x][cp.y] > MAX_HEIGHT) {
							ck->heights[cp.x][cp.y] = MAX_HEIGHT;
						} else if (ck->tiles[cp.x][cp.y] == tile_sea) {
							if (ck->heights[cp.x][cp.y] > 0.0f) {
								ck->tiles[cp.x][cp.y] = tile_coast;
							}
						} else if (ck->heights[cp.x][cp.y] < -0.2f) {
							ck->tiles[cp.x][cp.y] = tile_sea;
						}
						break;
					case terrain_mod_moisten: {
						if (ck->ent_height[cp.x][cp.y]) {
							continue;
						}

						uint32_t chance = 1000.0f * unit_distance_from_center;

						if (!rand_chance(chance * chance)) {
							continue;
						}

						touch_chunk(&sim->world->chunks, ck);

						if (ck->tiles[cp.x][cp.y] == tile_ash) {
							ck->tiles[cp.x][cp.y] = tile_rock;
						} else if (ck->tiles[cp.x][cp.y] == tile_rock) {
							ck->tiles[cp.x][cp.y] = tile_coast;
						} else if (ck->tiles[cp.x][cp.y] == tile_coast) {
							ck->tiles[cp.x][cp.y] = tile_plain;
						} else if (ck->tiles[cp.x][cp.y] == tile_plain) {
							ck->tiles[cp.x][cp.y] = tile_old_tree;
						} else if (ck->tiles[cp.x][cp.y] == tile_old_tree) {
							ck->tiles[cp.x][cp.y] = tile_tree;
						}
						break;
					}
					}
				}
			}
		}
	}
	TracyCZoneAutoE;
}

/* static int */
/* compare_ent_height(const void *_a, const void *_b) */
/* { */
/* 	const struct ent *a = *(struct ent **)_a, *b = *(struct ent **)_b; */

/* 	if (a->z == b->z) { */
/* 		return 0; */
/* 	} else if (a->z < b->z) { */
/* 		return -1; */
/* 	} else { */
/* 		return 1; */
/* 	} */
/* } */

/* static void */
/* hash_ent_pos(struct simulation *sim) */
/* { */
/* 	uint32_t i; */
/* 	struct ent *e; */

/* 	TracyCZoneAutoS; */

/* 	hash_clear(&sim->eb); */
/* 	darr_clear(&sim->ents_sorted); */

/* 	for (i = 0; i < sim->world->ents.darr.len; ++i) { */
/* 		e = hdarr_get_by_i(&sim->world->ents, i); */
/* 		darr_push(&sim->ents_sorted, &e); */
/* 	} */

/* 	qsort(sim->ents_sorted.e, sim->ents_sorted.len, sim->ents_sorted.item_size, compare_ent_height); */

/* 	for (i = 0; i < sim->ents_sorted.len; ++i) { */
/* 		e = *(struct ent **)darr_get(&sim->ents_sorted, i); */


/* 		/1* LOG_W(log_misc, "found bucket len:%d, flg:%d, next: %d", b->len, b->flags, b->next); *1/ */
/* 	} */

/* 	TracyCZoneAutoE; */
/* } */

static void
update_ent_positions(struct simulation *sim)
{
	hash_clear(&sim->eb);
	struct ent *e;
	uint32_t i;
	for (i = 0; i < sim->world->ents.darr.len; ++i) {
		e = hdarr_get_by_i(&sim->world->ents, i);

		const float terrain_height = get_height_at(&sim->world->chunks, &e->pos);
		if (e->real_pos[2] < terrain_height) {
			e->real_pos[2] = terrain_height;
		}

		struct point3d p = {
			.x = e->real_pos[0],
			.y = e->real_pos[1],
			.z = e->real_pos[2]
		};

		/* LOG_W(log_misc, "setting (%d, %d, %d), %d", key.x, key.y, key.z, e->id); */
		while (hash_get(&sim->eb, &p)) {
			e->real_pos[2] += 1.0f;
			++p.z;
		}

		uint64_t ptr = (uint64_t)e;
		hash_set(&sim->eb, &p, ptr);

		if (e->pos.x != p.x || e->pos.y != p.y || e->z != p.z) {
			e->pos.x = p.x;
			e->pos.y = p.y;
			e->z = p.z;
			e->state |= es_modified;
			e->modified |= eu_pos;
		}

		vec_scale(e->velocity, 0.90f);
		uint32_t j;
		for (j = 0; j < 3; ++j) {
			if (fabsf(e->velocity[j]) < 0.01f) {
				e->velocity[j] = 0.0f;
			}
		}
	}
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

	handle_player_actions(sim);

	update_ent_positions(sim);
	simulate_ents(sim);

	modify_terrain(sim);
	darr_clear(&sim->terrain_mods);

	update_player_counted_stats(sim);

	++sim->tick;

	hpa_clean(&sim->world->chunks);

	TracyCZoneAutoE;
}
