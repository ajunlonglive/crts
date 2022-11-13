#include "posix.h"

#include <stdlib.h>
#include <string.h>

#include "server/sim/ai.h"
#include "shared/constants/numbers.h"
#include "shared/math/rand.h"
#include "shared/sim/tiles.h"
#include "shared/util/log.h"

#define AI_COUNT 8
#define AI_RESPAWNS false

enum ai_ctx_flags {
	aif_init        = 1 << 0,
	aif_have_tgt    = 1 << 1,
	aif_have_tgt_pt = 1 << 2,
	aif_dead        = 1 << 3,
};

struct ai_ctx {
	uint16_t id, tgt_id, flags;
	int32_t search_r;
	struct point tgt_pt;
};
static uint16_t ai_id_seq = 0;

static struct player *
find_tgt(struct simulation *sim, struct player *ai)
{
	struct player *p;
	uint32_t i;
	for (i = 0; i < sim->players.len; ++i) {
		p = darr_get(&sim->players, i);
		if (p->ent_count && p->id != ai->id && p->id > 100) {
			return p;
		}
	}

	return NULL;
}

static void
move_cursor(struct player *aip, const struct point *diff)
{
	if (diff->x) {
		aip->cursor.x += (diff->x > 0 ? 1 : -1);
	}

	if (diff->y) {
		aip->cursor.y += (diff->y > 0 ? 1 : -1);
	}
}

static bool
find_nearest_tree(struct simulation *sim, struct point *p, int32_t r, int32_t or)
{
	int32_t rs, ors, sqdist;
	int16_t x, y;
	struct point q;

	ors = or * or;
	rs = r * r;

	for (x = -r; x < r; ++x) {
		for (y = -r; y < r; ++y) {
			sqdist = x * x + y * y;

			if (sqdist > ors && sqdist <= rs) {
				q.x = p->x + x;
				q.y = p->y + y;

				struct chunk *ck = get_chunk_at(&sim->world->chunks, &q);
				struct point rp = point_sub(&q, &ck->pos);

				if (ck->tiles[rp.x][rp.y] == tile_tree) {
					*p = q;
					return true;
				}
			}
		}
	}

	return false;
}

static void
ai_sim(struct simulation *sim, struct ai_ctx *ai)
{
	struct player *aip, *tgt;

	if (ai->flags & aif_dead) {
		if (AI_RESPAWNS) {
			ai->flags = 0;
		} else {
			return;
		}
	}

	if (ai->flags & aif_init) {
		aip = get_player(sim, ai->id);
		if (!aip->ent_count) {
			ai->flags |= aif_dead;
			L(log_ai, "ai %d died", ai->id);
			return;
		}
	} else {
		ai->id = ++ai_id_seq;
		aip = add_new_player(sim, ai->id);
		ai->flags |= aif_init;
	}

	if (ai->flags & aif_have_tgt) {
		tgt = get_player(sim, ai->tgt_id);
	} else {
		if (!(tgt = find_tgt(sim, aip))) {
			return;
		}
		ai->flags |= aif_have_tgt;
		ai->tgt_id = tgt->id;
	}

	if (!tgt->ent_count) {
		ai->flags &= ~aif_have_tgt;
		return;
	}

	/* uint32_t dist_from_center_of_mass = */
	/* 	square_dist(&aip->cursor, &aip->ent_center_of_mass); */

	struct point diff;

	if (aip->ent_count > tgt->ent_count * 0.4) {
		diff = point_sub(&tgt->ent_center_of_mass, &aip->cursor);
		move_cursor(aip, &diff);

		ai->flags &= ~aif_have_tgt_pt;
	} else {
		diff = point_sub(&aip->cursor, &tgt->ent_center_of_mass);
		if ((diff.x * diff.x + diff.y * diff.y) < 500) {
			ai->flags &= ~aif_have_tgt_pt;
			move_cursor(aip, &diff);
		} else {
			if (!(ai->flags & aif_have_tgt_pt)) {
				ai->tgt_pt = aip->ent_center_of_mass;

				while (ai->search_r < 25) {
					int32_t or = ai->search_r;
					++ai->search_r;
					if (find_nearest_tree(sim, &ai->tgt_pt, ai->search_r, or)) {
						diff = point_sub(&ai->tgt_pt, &tgt->ent_center_of_mass);
						if ((diff.x * diff.x + diff.y * diff.y) > 500) {
							assert(get_tile_at(&sim->world->chunks, &ai->tgt_pt) == tile_tree);
							ai->flags |= aif_have_tgt_pt;
							ai->search_r = 0;
							break;
						}
					}
				}
			}

			if (ai->flags & aif_have_tgt_pt) {
				diff = point_sub(&ai->tgt_pt, &aip->cursor);
				if (!diff.x && !diff.y) {
					ai->flags &= ~aif_have_tgt_pt;
				} else {
					move_cursor(aip, &diff);
				}
			}
		}
	}

	struct point p = nearest_chunk(&aip->cursor);
	struct chunk *ck = get_chunk(&sim->world->chunks, &p);
	p = point_sub(&aip->cursor, &p);
	float h = ck->heights[p.x][p.y];

	if (h < 0.0f) {
		aip->action = act_create;
	}
}

static struct ai_ctx ais[AI_COUNT];

void
ai_reset(void)
{
	memset(ais, 0, sizeof(struct ai_ctx) * AI_COUNT);
}

void
ai_tick(struct simulation *sim)
{
	return;

	uint32_t i;
	for (i = 0; i < AI_COUNT; ++i) {
		ai_sim(sim, &ais[i]);
	}
}
