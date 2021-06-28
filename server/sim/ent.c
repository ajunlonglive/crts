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

static void
change_alignment(struct ent *e, uint32_t newalign)
{
	e->alignment = newalign;
	e->state |= es_modified;
	e->modified |= eu_alignment;
}

void
kill_ent(struct simulation *sim, struct ent *e)
{
	struct ent *te;

	if (!(e->state & es_killed)) {
		update_tile_ent_height(sim->world, &e->pos, -1);

		destroy_ent(sim->world, e);

		if (gcfg.ents[e->type].corpse) {
			te = spawn_ent(sim->world);
			te->pos = e->pos;
			te->type = gcfg.ents[e->type].corpse;
		}
	}
}

void
damage_ent(struct simulation *sim, struct ent *e, uint8_t damage)
{
	/* TODO: check for overflow */
	if ((e->damage += damage) > gcfg.ents[e->type].hp) {
		kill_ent(sim, e);
	}
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

static void
ent_move(struct world *w, struct ent *e, int8_t diffx, int8_t diffy)
{
	if (!diffx && !diffy) {
		return;
	}

	struct point cur_cp = nearest_chunk(&e->pos);
	struct chunk *cur_ck = get_chunk(&w->chunks, &cur_cp), *dest_ck;

	struct point dest = { e->pos.x + diffx, e->pos.y + diffy };
	struct point dest_cp = nearest_chunk(&dest);
	if (points_equal(&cur_cp, &dest_cp)) {
		dest_ck = cur_ck;
	} else {
		dest_ck = get_chunk(&w->chunks, &dest_cp);
	}

	cur_cp = point_sub(&e->pos, &cur_ck->pos);
	dest_cp = point_sub(&dest, &dest_ck->pos);

	if (!(gcfg.tiles[dest_ck->tiles[dest_cp.x][dest_cp.y]].trav_type & e->trav)) {
		return;
	}

	float cur_top = (cur_ck->heights[cur_cp.x][cur_cp.y] + cur_ck->ent_height[cur_cp.x][cur_cp.y]);
	float height_diff = (dest_ck->heights[dest_cp.x][dest_cp.y] + dest_ck->ent_height[dest_cp.x][dest_cp.y])
			    - cur_top;

	if (height_diff < 1.0f) {
		--cur_ck->ent_height[cur_cp.x][cur_cp.y];
		++dest_ck->ent_height[dest_cp.x][dest_cp.y];

		e->pos = dest;
		e->state |= es_modified;
		e->modified |= eu_pos;
	}
}

static void
ent_meander(struct world *w, struct ent *e)
{
	uint8_t choice = rand_uniform(4);

	switch (choice) {
	case 0:
		ent_move(w, e, 1, 0);
		break;
	case 1:
		ent_move(w, e, -1, 0);
		break;
	case 2:
		ent_move(w, e, 0, 1);
		break;
	case 3:
		ent_move(w, e, 0, -1);
		break;
	}
}

static void
process_idle(struct simulation *sim, struct ent *e)
{
	TracyCZoneAutoS;
	if (rand_chance(gcfg.misc.meander_chance)) {
		ent_meander(sim->world, e);
	}
	TracyCZoneAutoE;
}

struct ent_collider_ctx {
	struct ent *e;
	struct player *p;
	struct simulation *sim;
};

enum iteration_result
ent_collider_cb(void *_ctx, struct ent *e)
{
	struct ent_collider_ctx *ctx = _ctx;
	if (!gcfg.ents[e->type].animate || e->state & es_killed) {
		return ir_cont;
	} else if (e->alignment == ctx->e->alignment) {
		return ir_cont;
	} else if (e->state & es_killed) {
		return ir_cont;
	}

	switch (ctx->p->action) {
	case act_create:
		if (e->loyalty) {
			--e->loyalty;
		} else {
			e->loyalty = 10;
			change_alignment(e, ctx->p->id);
		}
		break;
	case act_destroy:
		damage_ent(ctx->sim, e, 10);
		break;
	default:
		break;
	}

	return ir_done;
}

static const float height_mod = 0.001f;

static void
queue_terrain_mod(struct simulation *sim, struct point *pos, float dh)
{
	struct terrain_mod *tm;

	if ((tm = hdarr_get(&sim->terrain_mods, pos))) {
		tm->mod += dh;
		/* L(log_sim, "tm->mod: %f", tm->mod); */
	} else {
		/* L(log_sim, "tm->mod: (%d, %d), %f", pos->x, pos->y, dh); */

		hdarr_set(&sim->terrain_mods, pos, &(struct terrain_mod) {
			.pos = *pos,
			.mod = dh,
		});
	}
}

#define RADIUS_OF_INFLUENCE 5000

enum iteration_result
simulate_ent(void *_sim, void *_e)
{
	TracyCZoneAutoS;
	struct simulation *sim = _sim;
	struct ent *e = _e;
	uint32_t over_age;

	switch (get_tile_at(&sim->world->chunks, &e->pos)) {
	case tile_fire:
		damage_ent(sim, e, gcfg.misc.fire_damage);
		break;
	case tile_sea:
		if (e->trav != trav_aquatic) {
			/* kill_ent(sim, e); */
			/* struct ent *spawned = spawn_ent(sim->world); */
			/* spawned->pos = e->pos; */
			/* spawned->type = et_fish; */
		}
		break;
	default:
		break;
	}

	if (e->state & es_killed) {
		goto return_continue;
	} else if (!gcfg.ents[e->type].animate) {
		goto sim_age;
	}

	if (e->type == et_worker) {
		struct player *p;
		if (!e->loyalty) {
			goto return_continue;
		} else {
			p = get_player(sim, e->alignment);
			assert(p);
		}


		if (p->action != act_neutral) {
			for_each_ent_at(&sim->eb, &sim->world->ents, &e->pos, &(struct ent_collider_ctx) {
				.sim = sim,
				.p = p,
				.e = e
			}, ent_collider_cb);
		}

		++p->ent_count;
		p->ent_center_of_mass.x += e->pos.x;
		p->ent_center_of_mass.y += e->pos.y;

		/* struct point opos = e->pos; */

		uint32_t dist = square_dist(&p->cursor, &e->pos);
		if (dist >= RADIUS_OF_INFLUENCE) {
			goto return_continue;
		}

		struct point diff = point_sub(&p->cursor, &e->pos);

		if (abs(diff.x)) {
			ent_move(sim->world, e, diff.x > 0 ? 1 : -1, 0);
		}

		if (abs(diff.y)) {
			ent_move(sim->world, e, 0, diff.y > 0 ? 1 : -1);
		}

		struct chunk *ck = get_chunk_at(&sim->world->chunks, &e->pos);
		struct point rp = point_sub(&e->pos, &ck->pos);

		enum tile t = ck->tiles[rp.x][rp.y];

		switch (p->action) {
		case act_neutral:
			break;
		case act_create:
		{
			queue_terrain_mod(sim, &e->pos, height_mod);
			if (rand_chance(4)) {
				damage_ent(sim, e, 1);
			}

			switch (t) {
			case tile_old_tree:
				if (rand_chance(30)) {
					update_tile(sim->world, &e->pos, tile_dirt);
				}

				break;
			case tile_tree:
				if (rand_chance(40)) {
					update_tile(sim->world, &e->pos, tile_old_tree);
				}

				break;
			case tile_plain: {
				if (rand_chance(20)) {
					update_tile(sim->world, &e->pos, tile_tree);

					kill_ent(sim, e);
				}
				break;
			}
			case tile_dirt:
				update_tile(sim->world, &e->pos, tile_plain);
				break;
			default:
				break;
			}
		}
		break;
		case act_destroy:
		{
			switch (t) {
			case tile_tree:
			case tile_old_tree:
				update_tile(sim->world, &e->pos, tile_dirt);

				struct ent *new_ent = spawn_ent(sim->world);
				new_ent->type = et_worker;
				new_ent->pos = e->pos;
				new_ent->alignment = e->alignment;

				break;
			case tile_plain:
			case tile_coast:
				update_tile(sim->world, &e->pos, tile_dirt);
				break;
			case tile_dirt:
				queue_terrain_mod(sim, &e->pos, -height_mod);
				break;
			default:
				break;
			}
		}
		break;
		default:
			assert(false);
			break;
		}
	} else {
		process_idle(sim, e);
	}
sim_age:
	if (gcfg.ents[e->type].lifespan
	    && ++e->age >= gcfg.ents[e->type].lifespan) {
		if (gcfg.ents[e->type].animate) {
			over_age = ++e->age - gcfg.ents[e->type].lifespan;

			if (over_age < gcfg.misc.max_over_age
			    && (rand_chance(gcfg.misc.max_over_age - over_age))) {
				goto return_continue;
			}
		}

		kill_ent(sim, e);
	}

return_continue:
	TracyCZoneAutoE;

	return ir_cont;
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
