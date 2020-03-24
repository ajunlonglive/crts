#include <string.h>

#include "shared/sim/ent.h"
#include "shared/util/log.h"

void
ent_init(struct ent *e)
{
	memset(e, 0, sizeof(struct ent));

#ifdef CRTS_SERVER
	e->satisfaction = 100;
#endif
}

struct find_ent_ctx {
	const struct point *p;
	void *ctx;
	struct ent *e;
	find_ent_predicate pred;
	uint32_t dist;
};

static enum iteration_result
find_ent_iterator(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct find_ent_ctx *ctx = _ctx;
	uint32_t dist;

	if (ctx->pred(ctx->ctx, e)) {
		dist = square_dist(&e->pos, ctx->p);

		if (dist < ctx->dist) {
			ctx->dist = dist;
			ctx->e = e;
		}
	}

	return ir_cont;
}

struct ent *
find_ent(const struct world *w, const struct point *p, void *ctx, find_ent_predicate epred)
{
	struct find_ent_ctx fectx = {
		.p = p,
		.ctx = ctx,
		.e = NULL,
		.pred = epred,
		.dist = UINT32_MAX
	};

	hdarr_for_each(w->ents, &fectx, find_ent_iterator);

	return fectx.e;
}
