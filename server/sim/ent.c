#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "server/sim/ent.h"
#include "server/sim/update_tile.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
#include "shared/math/linalg.h"
#include "shared/math/rand.h"
#include "shared/sim/ent.h"
#include "shared/sim/tiles.h"
#include "shared/types/result.h"
#include "shared/util/log.h"
#include "tracy.h"

void
destroy_ent(struct world *w, struct ent *e)
{
	if (!(e->state & es_killed)) {
		darr_push(&w->graveyard, &e->id);

		e->state |= (es_killed | es_modified);
	}
}

void
kill_ent(struct simulation *sim, struct ent *e)
{
	if (!(e->state & es_killed)) {
		destroy_ent(sim->world, e);
	}
}

bool
damage_ent(struct simulation *sim, struct ent *e, uint8_t damage)
{
	/* TODO: check for overflow */
	if ((e->damage += damage) > gcfg.ents[e->type].hp) {
		kill_ent(sim, e);
		return true;
	}
	return false;
}

struct ent *
spawn_ent(struct world *world, enum ent_type t, const struct point *pos)
{
	struct ent *e = darr_get_mem(&world->spawn);
	ent_init(e);
	e->id = world->seq++;
	e->type = t;
	e->pos = *pos;

	float terrain_height = get_height_at(&world->chunks, pos);
	if (e->type == et_fire || e->type == et_wood || e->type == et_explosion) {
		e->z = terrain_height;
	} else {
		e->z = terrain_height + 32;
	}
	return e;
}

enum iteration_result
process_spawn_iterator(void *_s, void *_e)
{
	struct ent *ne, *e = _e;
	struct simulation *s = _s;

	hdarr_set(&s->world->ents, &e->id, e);
	ne = hdarr_get(&s->world->ents, &e->id);
	ne->real_pos[0] = ne->pos.x;
	ne->real_pos[1] = ne->pos.y;
	ne->real_pos[2] = ne->z;
	ne->state = es_modified | es_spawned;
	ne->trav = gcfg.ents[ne->type].trav;
	return ir_cont;
}

static inline struct chunk *
qget_chunk(struct chunks *cks, struct point *p)
{
	static struct point o = { INT32_MIN, INT32_MIN };
	static struct chunk *ck;

	if (!(o.x == p->x && o.y == p->y)) {
		ck = hdarr_get(&cks->hd, p);
		o = *p;
	}

	return ck;
}

static void
ent_move_gravity2(struct simulation *sim, struct ent *e, float friction, bool fluid)
{
	const float mass = 1.0f;
	const float fgravity = mass * -9.8f * sim->t;

	const struct point p = e->pos;
	const float terrain_height = get_height_at(&sim->world->chunks, &p);
	/* const float clearance = e->real_pos[2] - terrain_height; */

	/* L(log_sim, "vel: (%f, %f, %f), pos: (%f, %f, %f), terrain_height: %f, clearance: %f", */
	/* e->velocity[0], e->velocity[1], e->velocity[2], */
	/* e->real_pos[0], e->real_pos[1], e->real_pos[2], */
	/* terrain_height, */
	/* clearance */
	/* ); */

	if (e->real_pos[2] < terrain_height) {
		/* L(log_sim, "below ground by %f", terrain_height - e->real_pos[2]); */
		return;
	} else if (e->real_pos[2] - terrain_height < 1.0f) {
		// on ground
		/* L(log_sim, "on ground, %f", e->real_pos[2] - terrain_height); */

		float plane[3][3] = {
			{ p.x, p.y, terrain_height },
			{ p.x - 1, p.y,
			  get_height_at(&sim->world->chunks, &(struct point) { p.x - 1, p.y }) },
			{ p.x, p.y - 1,
			  get_height_at(&sim->world->chunks, &(struct point) { p.x, p.y - 1 }) },
		};

		float vec[3];
		calc_normal(plane[0], plane[1], plane[2], vec);
		vec_scale(vec, -fgravity);

		vec_add(e->velocity, vec);
		/* L(log_sim, "normal: (%f, %f, %f)", vec[0], vec[1], vec[2]); */
		e->velocity[2] += fgravity;

		// friction
		memcpy(vec, e->velocity, sizeof(float) * 3);
		vec_scale(vec, -friction);
		vec_add(e->velocity, vec);
	} else {
		e->velocity[2] += fgravity;
		/* L(log_sim, "grav: (%f, %f, %f)", 0.0, 0.0, fgravity); */
	}


	float newpos[3];
	memcpy(newpos, e->real_pos, sizeof(float) * 3);
	vec_add(newpos, e->velocity);

	if (fluid) {
		goto repos;
	}

	struct point3d p3 = { .x = newpos[0], .y = newpos[1], .z = newpos[2] };

	if (e->pos.x != p3.x || e->pos.y != p3.y || e->z != p3.z) {
		uint64_t *ptr;
		struct ent *other;
		if ((ptr = hash_get(&sim->eb, &p3))) {
			other = (struct ent *)(*ptr);
			float collision_normal[3];
			memcpy(collision_normal, e->real_pos, sizeof(float) * 3);
			vec_sub(collision_normal, other->real_pos);
			/* L(log_sim, "collision: (%f, %f, %f)", */
			/* 	collision_normal[0], collision_normal[1], collision_normal[2]); */
			vec_normalize(collision_normal);

			float relative_velocity[3];
			memcpy(relative_velocity, e->velocity, sizeof(float) * 3);
			vec_sub(relative_velocity, other->velocity);
			float v_rel_norm = vec_dot(relative_velocity, collision_normal);
			vec_scale(collision_normal, v_rel_norm);

			/* L(log_sim, "vel: (%f, %f, %f), pos: (%f, %f, %f), impact: (%f, %f, %f)", */
			/* 	e->velocity[0], e->velocity[1], e->velocity[2], */
			/* 	newpos[0], newpos[1], newpos[2], */
			/* 	collision_normal[0], collision_normal[1], collision_normal[2] */
			/* 	); */

			/* vec_sub(newpos, e->velocity); */
			/* vec_sub(e->velocity, collision_normal); */
			/* L(log_sim, "old pos: (%d, %d, %d) | new pos: (%d, %d, %d) | fixed pos: (%d, %d, %d)", */
			/* 	e->pos.x, e->pos.y, e->z, */
			/* 	p3.x, p3.y, p3.z, */
			/* 	(int32_t)newpos[0], (int32_t)newpos[1], (int32_t)newpos[2]); */

			vec_add(other->velocity, collision_normal);
			/* } */
			/* } else { */
			e->velocity[0] = 0.0f;
			e->velocity[1] = 0.0f;
			e->velocity[2] = 0.0f;
			return;
			/* } */
		}
	}

repos:
	memcpy(e->real_pos, newpos, sizeof(float) * 3);
}

static void
ent_move_liquid(struct simulation *sim, struct ent *e, float friction)
{
	struct point3d p3 = { .x = e->pos.x, .y = e->pos.y, .z = e->z - 1 };
	if (hash_get(&sim->eb, &p3)) {
		p3.z += 1;

		const struct point3d neighbors[] = {
			{ -1,  0, 0, },
			{ +1,  0, 0, },
			{  0, -1, 0, },
			{  0, +1, 0, },
		};

		uint32_t i, j;
		for (i = 0; i < 4; ++i) {
			j = i; //(i + (sim->tick & 3)) & 3;

			p3.x = e->pos.x + neighbors[j].x;
			p3.y = e->pos.y + neighbors[j].y;

			if (hash_get(&sim->eb, &p3)) {
				continue;
			}

			e->velocity[0] += (float)neighbors[j].x * 0.1f;
			e->velocity[1] += (float)neighbors[j].y * 0.1f;
		}
	}

	ent_move_gravity2(sim, e, friction, true);
}

static void
explode(struct simulation *sim, const struct point3d *center, const uint16_t r)
{
	const float r3 = r * r * r;
	float dist3;
	int16_t x, y, z;
	struct point3d p3;

	uint64_t *ent_ptr;
	struct ent *e;

	for (z = -r; z <= r; ++z) {
		for (y = -r; y <= r; ++y) {
			for (x = -r; x <= r; ++x) {
				dist3 = x * y * z;
				if (dist3 > r3) {
					continue;
				}

				p3 = *center;
				p3.x += x;
				p3.y += y;
				p3.z += z;

				if (!(ent_ptr = hash_get(&sim->eb, &p3))) {
					continue;
				}

				e = (struct ent *)(*ent_ptr);

				float collision_normal[3] = { x, y, z };
				vec_normalize(collision_normal);
				vec_scale(collision_normal, 0.4f * (1.0f - fabsf(dist3 / r3)));
				vec_add(e->velocity, collision_normal);
				/* L(log_sim, "adding (%f, %f, %f)", */
				/* 	collision_normal[0], collision_normal[1], collision_normal[2]); */
			}
		}
	}

	darr_push(&sim->terrain_mods, &(struct terrain_mod) {
		.r = r,
		.pos = { .x = center->x, .y = center->y },
		.type = terrain_mod_crater,
		.mod.height = -0.001f,
	});
}

struct ent_collider_ctx {
	struct ent *e;
	struct simulation *sim;
};

enum iteration_result
ent_collider_cb(void *_ctx, struct ent *e)
{
	struct ent_collider_ctx *ctx = _ctx;
	struct simulation *sim = ctx->sim;

	if (e->type == et_wood) {
		kill_ent(sim, e);
		struct ent *new = spawn_ent(sim->world, et_fire, &e->pos);
		new->z = e->z;

		explode(sim, &(struct point3d) { e->pos.x, e->pos.y, e->z }, 9);
	}


	if (e->state & es_killed) {
		return ir_cont;
	}

	return ir_cont;
}

static void
ent_move_fire(struct simulation *sim, struct ent *e)
{
	struct point p = e->pos;
	struct ent_collider_ctx ec_ctx = { .sim = sim, .e = e };

	for_each_ent_adjacent_to(&sim->eb, &sim->world->ents, e, &ec_ctx, ent_collider_cb);

	struct point cp = nearest_chunk(&e->pos);
	struct chunk *ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&e->pos, &ck->pos);

	if (gcfg.tiles[ck->tiles[cp.x][cp.y]].flamable) {
		update_tile(sim->world, &e->pos, tile_ash);
	} else {
		if (rand_chance(5)) {
			kill_ent(sim, e);
		}
	}

	switch (rand_uniform(4)) {
	case 0: ++p.x; break;
	case 1: ++p.y; break;
	case 2: --p.x; break;
	case 3: --p.y; break;
	}

	cp = nearest_chunk(&p);
	ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&p, &ck->pos);

	if (ck->ent_height[cp.x][cp.y] == 0 && gcfg.tiles[ck->tiles[cp.x][cp.y]].flamable) {
		spawn_ent(sim->world, et_fire, &p);
	}
}

static enum iteration_result
ent_collider_acid_cb(void *_ctx, struct ent *e)
{
	struct ent_collider_ctx *ctx = _ctx;
	struct simulation *sim = ctx->sim;

	if (ctx->e == e) {
		return ir_cont;
	} else if (e->state & es_killed) {
		return ir_cont;
	}

	if (e->type != et_acid && rand_chance(3)) {
		kill_ent(sim, e);

		if (rand_chance(10)) {
			kill_ent(sim, ctx->e);
			return ir_done;
		}
	}

	return ir_cont;
}

static void
simulate_acid(struct simulation *sim, struct ent *e)
{
	struct ent_collider_ctx ec_ctx = { .sim = sim, .e = e };
	for_each_ent_adjacent_to(&sim->eb, &sim->world->ents, e, &ec_ctx, ent_collider_acid_cb);

	if (rand_chance(3)) {
		update_tile(sim->world, &e->pos, tile_rock);
		update_tile_height(sim->world, &e->pos, -0.01f);
		if (rand_chance(10)) {
			kill_ent(sim, e);
		}
	}

	ent_move_gravity2(sim, e, 0.1f, true);
}

static void
simulate_water(struct simulation *sim, struct ent *e)
{
	struct point cp;
	struct chunk *ck;

	cp = nearest_chunk(&e->pos);
	ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&e->pos, &ck->pos);

	if (e->real_pos[2] <= 0.0f) {
		kill_ent(sim, e);
		return;
	}

	/* ck->tiles[cp.x][cp.y]; */

	if (rand_chance(10)) {
		update_tile(sim->world, &e->pos, tile_coast);

		if (rand_chance(100)) {
			kill_ent(sim, e);
			darr_push(&sim->terrain_mods, &(struct terrain_mod) {
				.r = 30,
				.pos = e->pos,
				.type = terrain_mod_moisten,
			});
		}
	}

	ent_move_liquid(sim, e, 0.01f);
}

void
simulate_ents(struct simulation *sim)
{
	TracyCZoneAutoS;
	struct ent *e;
	uint32_t i;

	for (i = 0; i < sim->ents_sorted.len; ++i) {
		e = *(struct ent **)darr_get(&sim->ents_sorted, i);

		if (e->state & es_killed) {
			continue;
		}

		if (e->type == et_fire) {
			if (sim->tick & 3) {
				continue;
			}
			ent_move_fire(sim, e);
		} else if (e->type == et_sand) {
			ent_move_gravity2(sim, e, 0.25f, false);
		} else if (e->type == et_acid) {
			simulate_acid(sim, e);
		} else if (e->type == et_water) {
			simulate_water(sim, e);
		} else if (e->type == et_spring) {
			if (sim->tick & 15) {
				continue;
			}
			spawn_ent(sim->world, et_water, &e->pos);
		} else if (e->type == et_explosion) {
			explode(sim, &(struct point3d) { e->pos.x, e->pos.y, e->z }, 9);
			kill_ent(sim, e);
		}
	}
	TracyCZoneAutoE;
}

bool
ent_pgraph_set(struct chunks *cnks, struct ent *e, const struct point *g)
{
	if (hpa_start(cnks, &e->pos, g, &e->path)) {
		e->state |= es_pathfinding;
		return true;
	}

	return false;
}

enum result
ent_pathfind(struct chunks *cnks, struct ent *e)
{
	enum result r;

	assert(e->state & es_pathfinding);

	switch (r = hpa_continue(cnks, e->path, &e->pos)) {
	case rs_cont:
		e->state |= es_modified;
		e->modified |= eu_pos;
		break;
	case rs_fail:
	case rs_done:
		e->state &= ~es_pathfinding;
		break;
	}

	return r;
}
