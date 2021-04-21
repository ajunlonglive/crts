#include "posix.h"

#include <stdlib.h>

#include "server/sim/ai.h"
#include "shared/constants/numbers.h"
#include "shared/math/rand.h"
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
			L("ai target: %d", p->id);
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

void
ai_tick(struct simulation *sim)
{
	static struct ai_ctx _ai, *ai = &_ai;
	struct player *aip, *tgt;

	if (ai->flags & aif_init) {
		aip = get_player(sim, ai->id);
		if (!aip->ent_count) {
			ai->flags &= ~aif_init;
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

	struct point diff;

	if (aip->ent_count > tgt->ent_count) {
		diff = point_sub(&tgt->cursor, &aip->cursor);
		move_cursor(aip, &diff);
	} else {
		diff = point_sub(&aip->cursor, &tgt->cursor);
		if ((diff.x * diff.x + diff.y * diff.y) < 500) {
			move_cursor(aip, &diff);
		} else {
			if (!(ai->flags & aif_have_tgt_pt)) {
				ai->tgt_pt = aip->cursor;
				ai->tgt_pt.x += 5 - rand_uniform(10);
				ai->tgt_pt.y += 5 - rand_uniform(10);
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

	struct point p = nearest_chunk(&aip->cursor);
	struct chunk *ck = get_chunk(&sim->world->chunks, &p);
	p = point_sub(&aip->cursor, &p);
	float h = ck->heights[p.x][p.y];
	/* enum tile t = ck->tiles[p.x][p.y]; */

	if (h > 2.0) {
		aip->action = act_destroy;
	} else if (aip->ent_count > tgt->ent_count) {
		aip->action = act_create;
	} else {
		aip->action = act_neutral;
	}
}
