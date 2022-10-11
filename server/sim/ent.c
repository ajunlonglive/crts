#include "posix.h"

#include <stdlib.h>

#include "server/sim/ent.h"
#include "server/sim/update_tile.h"
#include "server/sim/worker.h"
#include "shared/constants/globals.h"
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
		/* update_tile_ent_height(sim->world, &e->pos, -1); */
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
spawn_ent(struct world *w)
{
	struct ent *e = darr_get_mem(&w->spawn);
	ent_init(e);
	e->loyalty = 12;
	e->id = w->seq++;

	return e;
}

enum iteration_result
process_spawn_iterator(void *_s, void *_e)
{
	struct ent *ne, *e = _e;
	struct simulation *s = _s;

	hdarr_set(&s->world->ents, &e->id, e);
	ne = hdarr_get(&s->world->ents, &e->id);
	ne->state = es_modified | es_spawned;
	ne->trav = gcfg.ents[ne->type].trav;

	update_tile_ent_height(s->world, &ne->pos, 1);
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

static float
total_height_at(struct simulation *sim, const struct point *p, struct point *cp, struct chunk **ck)
{
	*cp = nearest_chunk(p);
	*ck = qget_chunk(&sim->world->chunks, cp);
	*cp = point_sub(p, &(*ck)->pos);
	return (*ck)->heights[cp->x][cp->y] + (*ck)->ent_height[cp->x][cp->y];
}

static void
check_min_height(struct simulation *sim, struct ent *e, const struct point *p,
	float *mh,
	struct point *mp, struct point *mcp, struct chunk **mck,
	struct point *dest_cp, struct chunk **dest_ck)
{
	float h;

	if (*mh > (h = total_height_at(sim, p, dest_cp, dest_ck))) {
		*mcp = *dest_cp;
		*mck = *dest_ck;
		*mh = h;
		*mp = *p;
	}
}

static void
ent_move_gravity(struct simulation *sim, struct ent *e, float nbr, bool diag)
{
	struct chunk *cur_ck, *dest_ck, *mck = NULL;
	struct point cur_cp, dest_cp;
	float cur_h = total_height_at(sim, &e->pos, &cur_cp, &cur_ck) - nbr;

	struct point p = e->pos, mp, mcp;

	float mh = cur_h;

	p.y -= 1;

	uint32_t i;
	for (i = 0; i < 3; ++i) {
		p.x = e->pos.x - 1;

		if (i == 1 || diag) {
			check_min_height(sim, e, &p, &mh, &mp, &mcp, &mck, &dest_cp, &dest_ck);
		}
		++p.x;

		check_min_height(sim, e, &p, &mh, &mp, &mcp, &mck, &dest_cp, &dest_ck);
		++p.x;

		if (i == 1 || diag) {
			check_min_height(sim, e, &p, &mh, &mp, &mcp, &mck, &dest_cp, &dest_ck);
		}

		p.y += 1;
	}

	if (mck) {
		--cur_ck->ent_height[cur_cp.x][cur_cp.y];
		++mck->ent_height[mcp.x][mcp.y];
		e->pos = mp;
		e->state |= es_modified;
		e->modified |= eu_pos;
	}
}

struct ent_collider_ctx {
	struct ent *e;
	struct simulation *sim;
	bool adj;
};

enum iteration_result
ent_collider_cb(void *_ctx, struct ent *e)
{
	struct ent_collider_ctx *ctx = _ctx;
	struct simulation *sim = ctx->sim;

	if (ctx->e == e) {
		return ir_cont;
	} else if (e->state & es_killed) {
		return ir_cont;
	}

	if (ctx->adj) {
		if (e->type == et_wood && rand_chance(3)) {
			kill_ent(sim, e);
			struct ent *new_ent = spawn_ent(sim->world);
			new_ent->type = et_fire;
			new_ent->pos = e->pos;
		}
	} else {
		if (e->type == et_fire) {
			kill_ent(sim, e);
		}
	}

	return ir_cont;
}

static void
ent_move_fire(struct simulation *sim, struct ent *e)
{
	struct point p = e->pos;
	struct ent_collider_ctx ec_ctx = { .sim = sim, .e = e };

	for_each_ent_at(&sim->eb, &sim->world->ents, &e->pos, &ec_ctx, ent_collider_cb);
	ec_ctx.adj = true;
	for_each_ent_adjacent_to(&sim->eb, &sim->world->ents, &e->pos, &ec_ctx, ent_collider_cb);

	struct point cp = nearest_chunk(&e->pos);
	struct chunk *ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&e->pos, &ck->pos);

	if (gcfg.tiles[ck->tiles[cp.x][cp.y]].flamable) {
		update_tile(sim->world, &e->pos, tile_ash);
	} else {
		if (rand_chance(10)) {
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
		struct ent *new_ent = spawn_ent(sim->world);
		new_ent->type = et_fire;
		new_ent->pos = p;
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
	for_each_ent_at(&sim->eb, &sim->world->ents, &e->pos, &ec_ctx, ent_collider_acid_cb);
	ec_ctx.adj = true;
	for_each_ent_adjacent_to(&sim->eb, &sim->world->ents, &e->pos, &ec_ctx, ent_collider_acid_cb);

	if (rand_chance(3)) {
		update_tile(sim->world, &e->pos, tile_ash);
		update_tile_height(sim->world, &e->pos, -0.01f);
		if (rand_chance(10)) {
			kill_ent(sim, e);
		}
	}

	ent_move_gravity(sim, e, 1.0f, true);
}

static void
simulate_water(struct simulation *sim, struct ent *e)
{
	struct point cp;
	struct chunk *ck;

	cp = nearest_chunk(&e->pos);
	ck = qget_chunk(&sim->world->chunks, &cp);
	cp = point_sub(&e->pos, &ck->pos);

	if (ck->heights[cp.x][cp.y] <= 0.0f) {
		kill_ent(sim, e);
		return;
	}

	/* ck->tiles[cp.x][cp.y]; */

	if (rand_chance(10)) {
		update_tile(sim->world, &e->pos, tile_coast);
	}

	ent_move_gravity(sim, e, 0.8f, true);
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
			ent_move_fire(sim, e);
		} else if (e->type == et_sand) {
			if (sim->tick & 2) {
				return;
			}

			ent_move_gravity(sim, e, 1.0f, false);
		} else if (e->type == et_acid) {
			simulate_acid(sim, e);
		} else if (e->type == et_water) {
			simulate_water(sim, e);
		} else if (e->type == et_spring) {
			struct ent *new_ent = spawn_ent(sim->world);
			new_ent->type = et_water;
			new_ent->pos = e->pos;
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
