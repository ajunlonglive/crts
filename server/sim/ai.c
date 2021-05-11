#include "posix.h"

#include <stdlib.h>

#include "server/sim/ai.h"
#include "shared/constants/numbers.h"
#include "shared/math/rand.h"
#include "shared/sim/tiles.h"
#include "shared/util/log.h"

enum ai_ctx_flags {
	aif_init        = 1 << 0,
	aif_have_tgt    = 1 << 1,
	aif_have_tgt_pt = 1 << 2,
};

struct ai_ctx {
	uint16_t id, tgt_id, flags;
	struct point tgt_pt;
};

static struct player *
find_tgt(struct simulation *sim, struct player *ai)
{
	struct player *p;
	uint32_t i;
	for (i = 0; i < sim->players.len; ++i) {
		p = darr_get(&sim->players, i);
		if (p->ent_count && p->id > MIN_USER_ID) {
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

static void
find_nearest_tree(struct simulation *sim, struct point *p)
{
	uint32_t l = 1, cl = l;
	uint8_t d = 0, q = 0;

	while (l < 9) {
		if (!--cl) {
			if (++d == 4) {
				d = 0;
			}

			if (++q == 3) {
				++l;
				q = 0;
			}

			cl = l;
		}

		switch (d) {
		case 0:
			--p->y;
			break;
		case 1:
			--p->x;
			break;
		case 2:
			++p->y;
			break;
		case 3:
			++p->x;
			break;
		}

		struct chunk *ck = get_chunk_at(&sim->world->chunks, p);
		struct point rp = point_sub(p, &ck->pos);

		if (ck->tiles[rp.x][rp.y] == tile_tree
		    && ck->heights[rp.x][rp.y] > 2.0f) {
			break;
		}
	}
}

void
ai_tick(struct simulation *sim)
{
	static struct ai_ctx _ai, *ai = &_ai;
	struct player *aip, *tgt;

	if (ai->flags & aif_init) {
		aip = get_player(sim, ai->id);
		if (!aip->ent_count) {
			ai->flags = 0;
			return;
		}
	} else {
		++ai->id;
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

	uint32_t dist_from_center_of_mass =
		square_dist(&aip->cursor, &aip->ent_center_of_mass);

	/* L(log_misc, "com: %d, %d | dist: %d", aip->ent_center_of_mass.x, */
	/* 	aip->ent_center_of_mass.x, dist_from_center_of_mass); */

	if (dist_from_center_of_mass < 100) {
		struct point diff;

		if (aip->ent_count > tgt->ent_count * 2) {
			diff = point_sub(&tgt->cursor, &aip->cursor);
			move_cursor(aip, &diff);
		} else {
			diff = point_sub(&aip->cursor, &tgt->cursor);
			if ((diff.x * diff.x + diff.y * diff.y) < 500) {
				move_cursor(aip, &diff);
			} else {
				if (!(ai->flags & aif_have_tgt_pt)) {
					ai->tgt_pt = aip->cursor;
					find_nearest_tree(sim, &ai->tgt_pt);
					ai->flags |= aif_have_tgt_pt;
				}

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
	enum tile t = ck->tiles[p.x][p.y];

	if (t == tile_tree) {
		aip->action = act_destroy;
	} else if (aip->ent_count > 100 && h < 1.0f) {
		aip->action = act_create;
	} else {
		aip->action = act_neutral;
	}
}
