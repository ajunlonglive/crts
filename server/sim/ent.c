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
#include "shared/util/util.h"
#include "tracy.h"

void
destroy_ent(struct world *w, struct ent *e)
{
	assert(!(e->state & es_killed) && "double-kill");

	if (!(e->state & es_killed)) {
		darr_push(&w->graveyard, &e->id);

		e->state |= (es_killed | es_modified);
	}
}

void
kill_ent(struct simulation *sim, struct ent *e)
{
	assert(!(e->state & es_killed) && "double-kill");

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

	/* float terrain_height = get_height_at(&world->chunks, pos); */
	/* if (e->type == et_fire || e->type == et_wood || e->type == et_explosion) { */
	/* 	e->z = terrain_height; */
	/* } else { */
	/* 	e->z = terrain_height + 32; */
	/* } */
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
check_collision(struct simulation *sim, struct ent *e, vec3 newpos, float bounce)
{
	struct point3d p3 = { .x = newpos[0], .y = newpos[1], .z = newpos[2] };
	uint64_t *ptr; struct ent *o;
	if ((ptr = hash_get(&sim->eb, &p3)) && e != (struct ent *)*ptr) {
		o = (struct ent *)*ptr;

		vec3 dpos = { 0 }, ei = { 0 }, oi = { 0 };
		vec_add(dpos, o->real_pos);
		vec_sub(dpos, e->real_pos);

		vec_normalize(dpos);

		vec_add(ei, e->real_pos);
		vec_sub(ei, dpos);

		vec_add(oi, o->real_pos);
		vec_add(oi, dpos);

		/* L(log_cli, "dpos, %f, %f, %f, ei: %f, %f, %f, oi: %f, %f, %f", */
		/* 	dpos[0], dpos[1], dpos[2], */
		/* 	ei[0], ei[1], ei[2], oi[0], oi[1], oi[2]); */

		vec_sub(ei, oi);

		/* L(log_cli, "intersection: %f, %f, %f", ei[0], ei[1], ei[2]); */
		vec_scale(ei, bounce);

		/* vec3 rel_velocity = { 0 }; */
		/* vec_add(rel_velocity, e->velocity); */
		/* vec_sub(rel_velocity, o->velocity); */

		memcpy(e->velocity, ei, sizeof(vec3));

		/* vec_scale(ei, -1); */
		/* memcpy(o->velocity, ei, sizeof(vec3)); */
	}
}

static void
apply_friction(vec3 velocity, float mu)
{
	vec3 ffriction;
	memcpy(ffriction, velocity, sizeof(vec3));
	vec_scale(ffriction, -mu);
	vec_add(velocity, ffriction);
}

static void
ent_move_gas(struct simulation *sim, struct ent *e, float weight)
{
	const float max_height = 100.0f;
	float unit_height = (fclamp(e->real_pos[2], 0.0f, max_height) / max_height);

	struct point np = nearest_chunk(&e->pos);
	vec3 *chunk_wind;
	if ((chunk_wind = hdarr_get(&sim->world->chunks.wind, &np))) {
		vec3 w = { 0 };
		vec_add(w, *chunk_wind);
		vec_scale(w, sim->t);
		vec_add(e->velocity, w);
	}

	e->velocity[2] += weight * (1.0 - unit_height) * sim->t;

	vec3 newpos = { 0 };
	vec_add(newpos, e->real_pos);
	vec_add(newpos, e->velocity);
	check_collision(sim, e, newpos, 0.1f);

	apply_friction(e->velocity, 0.1f);
}

static void
ent_move_gravity2(struct simulation *sim, struct ent *e, float friction)
{
	const float fgravity = -9.8f * sim->t;

	const struct point p = e->pos;
	const float terrain_height = get_height_at(&sim->world->chunks, &p);

	if (e->real_pos[2] - terrain_height < 1.0f) {
		// on ground
		/* L(log_sim, "on ground, %f", e->real_pos[2] - terrain_height); */

		float plane[3][3] = {
			{ p.x, p.y, terrain_height },
			{ p.x - 1, p.y,
			  get_height_at(&sim->world->chunks, &(struct point) { p.x - 1, p.y }) },
			{ p.x, p.y - 1,
			  get_height_at(&sim->world->chunks, &(struct point) { p.x, p.y - 1 }) },
		};

		vec3 normal;
		calc_normal(plane[0], plane[1], plane[2], normal);
		vec_normalize(normal);
		vec_scale(normal, -fgravity);
		/* for (uint32_t i = 0; i < 3; ++i) { */
		/* 	if (normal[i] < 0.0001f) { */
		/* 		normal[i] = 0.0f; */
		/* 	} */
		/* } */

		vec_add(e->velocity, normal);
		/* L(log_sim, "normal: (%f, %f, %f)", vec[0], vec[1], vec[2]); */
		e->velocity[2] += fgravity;

		apply_friction(e->velocity, friction);
	} else {
		e->velocity[2] += fgravity;
		apply_friction(e->velocity, 0.01f);
		/* L(log_sim, "grav: (%f, %f, %f)", 0.0, 0.0, fgravity); */
	}


	vec3 newpos = { 0 };
	vec_add(newpos, e->real_pos);
	vec_add(newpos, e->velocity);
	check_collision(sim, e, newpos, 0.0f);

	/* vec_add(e->real_pos, e->velocity); */
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

	ent_move_gravity2(sim, e, friction);
}

static void
explode(struct simulation *sim, const struct point3d *center, const uint16_t r, float force)
{
	const float rs = r * r;
	float dist3;
	int16_t x, y, z;
	struct point3d p3;

	/* uint64_t *ent_ptr; */
	struct ent *e;

	for (z = -r; z <= r; ++z) {
		for (y = -r; y <= r; ++y) {
			for (x = -r; x <= r; ++x) {
				dist3 = x * x + y * y + z * z;
				if (dist3 > rs) {
					continue;
				}

				p3 = *center;
				p3.x += x;
				p3.y += y;
				p3.z += z;

				float unit_distance_from_center = dist3 / rs;

				if (unit_distance_from_center < 0.4f && rand_chance(5)) {
					e = spawn_ent(sim->world, et_fire, &(struct point) { p3.x, p3.y });
					e->z = p3.z;
				}
			}
		}
	}

	darr_push(&sim->force_fields, &(struct force_field){
		.pos = { center->x, center->y, center->z },
		.rsq = r * r,
		.force = force,
	});

	darr_push(&sim->terrain_mods, &(struct terrain_mod) {
		.r = r,
		.pos = *center,
		.type = terrain_mod_crater,
		.mod.height = 0.00f,
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

	if (e->state & es_killed) {
		return ir_cont;
	}

	if (e->type == et_wood) {
		// nothing
	} else if (e->type == et_bomb) {
		explode(sim, &(struct point3d) { e->pos.x, e->pos.y, e->z }, 10, 2.0f);
	} else {
		return ir_cont;
	}

	kill_ent(sim, e);
	struct ent *new = spawn_ent(sim->world, et_fire, &e->pos);
	new->z = e->z;
	return ir_cont;
}

static void
ent_move_fire(struct simulation *sim, struct ent *e)
{
	struct point p = e->pos;
	struct ent_collider_ctx ec_ctx = { .sim = sim, .e = e };

	if (e->real_pos[2] <= 0 || e->z <= 0) {
		kill_ent(sim, e);
		return;
	}

	if (rand_chance(4)) {
		for_each_ent_adjacent_to(&sim->eb, e, &ec_ctx, ent_collider_cb);
	}

	struct point cp = nearest_chunk(&e->pos);
	struct chunk *ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&e->pos, &ck->pos);

	if (e->real_pos[2] - ck->heights[cp.x][cp.y] < 1.0f) {
		if (gcfg.tiles[ck->tiles[cp.x][cp.y]].flamable) {
			update_tile(sim->world, &e->pos, tile_ash);
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
		if (gcfg.tiles[ck->tiles[cp.x][cp.y]].flamable) {
			spawn_ent(sim->world, et_fire, &p);
		}
	}
	if (rand_chance(15)) {
		spawn_ent(sim->world, et_smoke, &e->pos)->z = e->z + 2;
	}

	if (rand_chance(5)) {
		kill_ent(sim, e);
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
	for_each_ent_adjacent_to(&sim->eb, e, &ec_ctx, ent_collider_acid_cb);

	if (rand_chance(3)) {
		update_tile(sim->world, &e->pos, tile_rock);
		update_tile_height(sim->world, &e->pos, -0.01f);
		if (rand_chance(10)) {
			kill_ent(sim, e);
		}
	}

	ent_move_gravity2(sim, e, 0.1f);
}

static void
simulate_water(struct simulation *sim, struct ent *e)
{
	struct point cp;
	struct chunk *ck;

	cp = nearest_chunk(&e->pos);
	ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&e->pos, &ck->pos);

	ent_move_liquid(sim, e, 0.1f);

	if (e->real_pos[2] <= 0.0f) {
		kill_ent(sim, e);
		return;
	}

	/* ck->tiles[cp.x][cp.y]; */

	if (rand_chance(100)) {
		darr_push(&sim->terrain_mods, &(struct terrain_mod) {
			.r = 10,
			.pos = { e->pos.x, e->pos.y },
			.type = terrain_mod_moisten,
		});

		if (rand_chance(40)) {
			kill_ent(sim, e);
			return;
		}
	}
}

void
simulate_ents(struct simulation *sim)
{
	TracyCZoneAutoS;
	struct ent *e;
	uint32_t i;
	for (i = 0; i < sim->world->ents.darr.len; ++i) {
		e = hdarr_get_by_i(&sim->world->ents, i);

		if (e->state & es_killed) {
			continue;
		}

		if (e->type == et_fire) {
			ent_move_gas(sim, e, 0.6f);
			if (sim->tick & 3) {
				continue;
			}
			ent_move_fire(sim, e);
		} else if (e->type == et_smoke) {
			if (e->age > 25 && rand_chance((100 - clamp(e->z, 0, 100)) * 3)) {
				kill_ent(sim, e);
			}
			ent_move_gas(sim, e, 2.0f);
		} else if (e->type == et_sand) {
			ent_move_gravity2(sim, e, 0.25f);
		} else if (e->type == et_acid) {
			simulate_acid(sim, e);
		} else if (e->type == et_water) {
			simulate_water(sim, e);
		} else if (e->type == et_spring) {
			if (sim->tick & 15) {
				continue;
			}
			struct point p = e->pos;
			p.x += 1;
			spawn_ent(sim->world, et_water, &p)->z = e->z;
		} else if (e->type == et_explosion) {
			explode(sim, &(struct point3d) { e->pos.x, e->pos.y, e->z }, 10, 2.0f);
			kill_ent(sim, e);
		} else if (e->type == et_push) {
			darr_push(&sim->force_fields, &(struct force_field){
				.pos = { e->real_pos[0], e->real_pos[1], e->real_pos[2], },
				.rsq = 100,
				.force = 1.0f,
			});
			kill_ent(sim, e);
		} else if (e->type == et_pull) {
			darr_push(&sim->force_fields, &(struct force_field){
				.pos = { e->real_pos[0], e->real_pos[1], e->real_pos[2], },
				.rsq = 100,
				.force = -0.5f,
				.black_hole = true,
			});
			kill_ent(sim, e);
		} else if (e->type == et_dampener) {
			darr_push(&sim->force_fields, &(struct force_field){
				.pos = { e->real_pos[0], e->real_pos[1], e->real_pos[2], },
				.rsq = 25,
				.force = 0.5f,
				.constant = true,
			});
		} else if (e->type == et_accelerator) {
			darr_push(&sim->force_fields, &(struct force_field){
				.pos = { e->real_pos[0], e->real_pos[1], e->real_pos[2], },
				.rsq = 25,
				.force = 1.01f,
				.constant = true,
			});
		}

		++e->age;
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
