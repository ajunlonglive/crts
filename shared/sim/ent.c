#include <string.h>

#include "shared/sim/alignment.h"
#include "shared/sim/ent.h"
#include "shared/util/log.h"

void
ent_init(struct ent *e)
{
	memset(e, 0, sizeof(struct ent));

#ifdef CRTS_SERVER
	e->alignment = alignment_init();
	e->satisfaction = 100;
	e->idle = true;
#endif
}

struct ent *
find_ent(const struct world *w, const struct point *p, void *ctx, find_ent_predicate epred)
{
	size_t i;
	uint32_t dist, closest_dist = UINT32_MAX;
	struct ent *e, *ret = NULL;

	for (i = 0; i < w->ents.len; i++) {
		e = &w->ents.e[i];

		if (epred(ctx, e)) {
			dist = square_dist(&e->pos, p);

			if (dist < closest_dist) {
				closest_dist = dist;
				ret = e;
			}
		}
	}

	return ret;
}
