#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#define _XOPEN_SOURCE 500

#include "server/sim/action.h"
#include "server/sim/do_action.h"
#include "server/sim/do_action/build.h"
#include "server/sim/do_action/fight.h"
#include "server/sim/do_action/harvest.h"
#include "server/sim/do_action/move.h"
#include "server/sim/terrain.h"
#include "shared/messaging/server_message.h"
#include "shared/types/result.h"
#include "shared/util/log.h"

static bool
find_resource_pred(void *ctx, struct ent *e)
{
	enum ent_type *et = ctx;

	return *et == e->type;
}

struct ent *
find_resource(struct world *w, enum ent_type t, struct point *p)
{
	return find_ent(w, p, &t, find_resource_pred);
}

bool
find_adj_tile(struct chunks *cnks, struct point *s, struct point *rp,
	struct circle *circ, enum tile t, bool (*pred)(enum tile t))
{
	enum tile tt;
	struct point p[4] = {
		{ s->x + 1, s->y     },
		{ s->x - 1, s->y     },
		{ s->x,     s->y + 1 },
		{ s->x,     s->y - 1 },
	};
	size_t i;

	for (i = 0; i < 4; ++i) {
		if (circ && !point_in_circle(&p[i], circ)) {
			continue;
		}

		tt = get_tile_at(cnks, &p[i]);

		if (tt == t || (pred && pred(tt))) {
			*rp = p[i];
			return true;
		}
	}

	return false;
}

enum result
do_action(struct simulation *sim, struct ent *e, struct sim_action *act)
{
	switch (act->act.type) {
	case at_harvest:
		return do_action_harvest(sim, e, act);
	case at_build:
		return do_action_build(sim, e, act);
	case at_move:
		return do_action_move(sim, e, act);
	case at_fight:
		return do_action_fight(sim, e, act);
	default:
		return rs_done;
	}
}
