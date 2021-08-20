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
		if (gcfg.ents[e->type].animate) {
			update_tile_ent_height(sim->world, &e->pos, -1);
		}

		destroy_ent(sim->world, e);

		if (gcfg.ents[e->type].corpse) {
			te = spawn_ent(sim->world);
			te->pos = e->pos;
			te->type = gcfg.ents[e->type].corpse;
		}
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

	if (gcfg.ents[e->type].animate) {
		update_tile_ent_height(s->world, &ne->pos, 1);
	}

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
ent_move(struct simulation *sim, struct ent *e, int8_t diffx, int8_t diffy)
{
	bool moved = false;
	float cur_top, dest_top;
	struct point dest, cur_cp, dest_cp;
	struct chunk *cur_ck, *dest_ck;

	dest = e->pos;

	/* LOG_W(log_misc, "moving %d, %d by %d, %d", e->pos.x, e->pos.x, diffx, diffy); */

	cur_cp = nearest_chunk(&e->pos);
	cur_ck = qget_chunk(&sim->world->chunks, &cur_cp);
	cur_cp = point_sub(&e->pos, &cur_ck->pos);
	cur_top = (cur_ck->heights[cur_cp.x][cur_cp.y] + cur_ck->ent_height[cur_cp.x][cur_cp.y]);

	if (!diffx) {
		goto dest_2;
	}

	dest.x += diffx;
	dest_cp = nearest_chunk(&dest);
	dest_ck = qget_chunk(&sim->world->chunks, &dest_cp);
	dest_cp = point_sub(&dest, &dest_ck->pos);

	if (!(gcfg.tiles[dest_ck->tiles[dest_cp.x][dest_cp.y]].trav_type & e->trav)) {
		dest.x -= diffx;
		goto dest_2;
	}

	dest_top = (dest_ck->heights[dest_cp.x][dest_cp.y] + dest_ck->ent_height[dest_cp.x][dest_cp.y]);

	if ((dest_top - cur_top) < 1.0f) {
		moved = true;
		--cur_ck->ent_height[cur_cp.x][cur_cp.y];
		++dest_ck->ent_height[dest_cp.x][dest_cp.y];

		cur_cp = dest_cp;
		cur_ck = dest_ck;
		cur_top = dest_top + 1;
	} else {
		dest.x -= diffx;
	}

dest_2:
	if (!diffy) {
		goto done;
	}

	dest.y += diffy;
	dest_cp = nearest_chunk(&dest);
	dest_ck = qget_chunk(&sim->world->chunks, &dest_cp);
	dest_cp = point_sub(&dest, &dest_ck->pos);

	if (!(gcfg.tiles[dest_ck->tiles[dest_cp.x][dest_cp.y]].trav_type & e->trav)) {
		dest.y -= diffy;
		goto done;
	}

	dest_top = (dest_ck->heights[dest_cp.x][dest_cp.y] + dest_ck->ent_height[dest_cp.x][dest_cp.y]);

	if ((dest_top - cur_top) < 1.0f) {
		moved = true;
		--cur_ck->ent_height[cur_cp.x][cur_cp.y];
		++dest_ck->ent_height[dest_cp.x][dest_cp.y];
	} else {
		dest.y -= diffy;
	}

done:
	if (moved) {
		e->pos = dest;
		e->state |= es_modified;
		e->modified |= eu_pos;
	}
}

static void
ent_meander(struct simulation *sim, struct ent *e)
{
	uint8_t choice = rand_uniform(4);

	switch (choice) {
	case 0:
		ent_move(sim, e, 1, 0);
		break;
	case 1:
		ent_move(sim, e, -1, 0);
		break;
	case 2:
		ent_move(sim, e, 0, 1);
		break;
	case 3:
		ent_move(sim, e, 0, -1);
		break;
	}
}

static void
process_idle(struct simulation *sim, struct ent *e)
{
	TracyCZoneAutoS;
	if (rand_chance(gcfg.misc.meander_chance)) {
		ent_meander(sim, e);
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

#define UPDATE_TILE(t, ck, new) *t = new; ck->touched_this_tick = true;

void
simulate_ents(struct simulation *sim)
{
	TracyCZoneAutoS;
	struct ent *e;
	uint32_t i;

	for (i = 0; i < sim->world->ents.darr.len; ++i) {
		e = hdarr_get_by_i(&sim->world->ents, i);
		uint32_t over_age;

#if 0
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
#endif

		if (e->state & es_killed) {
			continue;
		} else if (!gcfg.ents[e->type].animate) {
			goto sim_age;
		}

		if (e->type == et_worker) {
			struct player *p;
			if (!e->loyalty) {
				continue;
			} else {
				p = get_player(sim, e->alignment);
				assert(p);
			}

			++p->ent_count;
			p->ent_center_of_mass.x += e->pos.x;
			p->ent_center_of_mass.y += e->pos.y;

			/* struct point opos = e->pos; */

			uint32_t dist = square_dist(&p->cursor, &e->pos);
			if (dist >= RADIUS_OF_INFLUENCE) {
				continue;
			}

			struct point diff = point_sub(&p->cursor, &e->pos);
			int8_t diffy = 0, diffx = 0;

			diffx = diff.x ? (diff.x > 0 ? 1 : -1) : 0;
			diffy = diff.y ? (diff.y > 0 ? 1 : -1) : 0;

			if (diffx || diffy) {
				ent_move(sim, e, diffx, diffy);

				if (diff.x > diff.y) {
					diffy = 0;
				} else {
					diffx = 0;
				}
			}

			diff = e->pos;
			diff.x += diffx;
			diff.y += diffy;

			if (p->action != act_neutral) {
				for_each_ent_at(&sim->eb, &sim->world->ents, &diff, &(struct ent_collider_ctx) {
					.sim = sim,
					.p = p,
					.e = e
				}, ent_collider_cb);
			}

			struct chunk *ck = get_chunk_at(&sim->world->chunks, &e->pos);
			struct point rp = point_sub(&e->pos, &ck->pos);

			uint8_t *t = &ck->tiles[rp.x][rp.y];

			switch (p->action) {
			case act_neutral:
				break;
			case act_create:
			{
				queue_terrain_mod(sim, &e->pos, height_mod);
				if (rand_chance(4)) {
					damage_ent(sim, e, 1);
				}

				switch ((enum tile)*t) {
				case tile_old_tree:
					if (rand_chance(30)) {
						UPDATE_TILE(t, ck, tile_dirt);
					}

					break;
				case tile_tree:
					if (rand_chance(40)) {
						UPDATE_TILE(t, ck, tile_old_tree);
					}

					break;
				case tile_plain: {
					if (rand_chance(20)) {
						UPDATE_TILE(t, ck, tile_tree);

						kill_ent(sim, e);
					}
					break;
				}
				case tile_dirt:
					UPDATE_TILE(t, ck, tile_plain);
					break;
				default:
					break;
				}
			}
			break;
			case act_destroy:
			{
				switch (*t) {
				case tile_tree:
				case tile_old_tree:
					UPDATE_TILE(t, ck, tile_dirt);

					struct ent *new_ent = spawn_ent(sim->world);
					new_ent->type = et_worker;
					new_ent->pos = e->pos;
					new_ent->alignment = e->alignment;

					break;
				case tile_plain:
				case tile_coast:
					UPDATE_TILE(t, ck, tile_dirt);
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
					continue;
				}
			}

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
