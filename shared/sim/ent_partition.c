#include <stdlib.h>

#include "shared/sim/ent.h"
#include "shared/sim/ent_partition.h"
#include "shared/sim/world.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"
#include "shared/util/log.h"

struct ent_partition {
	struct hdarr *part;
	size_t psize;
};

struct ents_at_ctx {
	struct ent *ent;
	struct point p;
};

static enum iteration_result
ents_at_iterator(void *_eac, void *_ent)
{
	struct ents_at_ctx *eac = _eac;
	struct ent *ent = _ent;

	if (points_equal(&eac->p, &ent->pos)) {
		eac->ent = ent;
		return ir_done;
	}

	return ir_cont;
}

struct ent *
ep_get_ent_at(struct ent_partition *ep, const struct point *p)
{
	struct point np = point_mod(p, ep->psize);
	struct darr *part;

	if ((part = hdarr_get(ep->part, &np)) == NULL) {
		return NULL;
	}

	struct ents_at_ctx eac = {
		.ent = NULL,
		.p = *p
	};

	darr_for_each(part, &eac, ents_at_iterator);

	return eac.ent;
}

struct ent_partition *
ent_partition_init(const struct world *w, uint16_t partition_size)
{
	struct ent_partition *ep = calloc(1, sizeof(struct ent_partition));
	/*
	   size_t i;
	   struct darr *rgn, **rgnp;
	   struct point p;

	   ep->psize = partition_size;
	   ep->part = hdarr_init(1024, sizeof(struct point), sizeof(struct darr *), NULL);

	   int rgns = 0;

	   for (i = 0; i < w->ents.len; i++) {
	        p = point_mod(&w->ents.e[i].pos, partition_size);

	        if ((rgn = hdarr_get(ep->part, &p)) == NULL) {
	                rgn = darr_init(sizeof(struct ent *));
	                hdarr_set(ep->part, &p, &rgn);
	                rgnp = hdarr_get(ep->part, &p);
	                rgns++;
	        }

	        darr_push(*rgnp, &w->ents.e[i]);
	   }

	 */
	return ep;
}

static enum iteration_result
partition_destroy(void *_, void *_elem)
{
	struct darr **elem = _elem;

	darr_destroy(*elem);

	return ir_cont;
}

void
ent_partition_destroy(struct ent_partition *ep)
{
	hdarr_for_each(ep->part, NULL, partition_destroy);
	hdarr_destroy(ep->part);
	free(ep);
}
