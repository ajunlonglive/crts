#include "shared/sim/ent_buckets.h"
#include "server/sim/ent_lookup.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"

struct nearest_applicable_ent_iter_ctx {
	struct ent *ret;
	const struct point *origin;
	ent_lookup_pred pred;
	void *ctx;
	uint32_t min_dist;
};

static enum iteration_result
nearest_applicable_ent_iter(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct nearest_applicable_ent_iter_ctx *ctx = _ctx;
	uint32_t dist;

	if (ctx->pred(e, ctx->ctx)
	    && (dist = square_dist(&e->pos, ctx->origin)) < ctx->min_dist) {
		ctx->min_dist = dist;
		ctx->ret = e;
	}

	return ir_cont;
}

static struct ent *
nearest_applicable_ent(struct simulation *sim, struct nearest_applicable_ent_iter_ctx *ctx)
{
	ctx->ret = NULL;
	ctx->min_dist = UINT32_MAX;

	hdarr_for_each(sim->world->ents, ctx, nearest_applicable_ent_iter);

	return ctx->ret;
}

struct ascb_ctx {
	struct simulation *sim;
	ent_lookup_pred pred;
	ent_lookup_cb cb;
	void *ctx;
	uint16_t found;
	uint16_t needed;
	uint32_t checked;
	uint32_t total;
};

static enum iteration_result
check_workers_at(void *_ctx, struct ent *e)
{
	struct ascb_ctx *ctx = _ctx;

	if (ctx->pred(e, ctx->ctx)) {
		ctx->cb(e, ctx->ctx);

		if (++ctx->found >= ctx->needed) {
			return ir_done;
		}

		++ctx->checked;
	}

	return ir_cont;
}

static enum result
ascb(void *_ctx, const struct point *p)
{
	struct ascb_ctx *ctx = _ctx;

	for_each_ent_at(&ctx->sim->eb, ctx->sim->world->ents, p,
		ctx, check_workers_at);

	if (ctx->found >= ctx->needed || ctx->checked >= ctx->total) {
		return rs_done;
	} else {
		return rs_cont;
	}
}

uint16_t
ent_lookup(struct simulation *sim, struct pgraph *pg,
	void *usr_ctx, ent_lookup_pred pred,
	ent_lookup_cb cb, uint16_t needed, const struct point *origin)
{
	struct ent *e;

	//set_action_targets(sa);

	struct ascb_ctx ctx = {
		.sim = sim,
		.pred = pred,
		.cb = cb,
		.ctx = usr_ctx,
		.found = 0,
		.needed = needed,
		.checked = 0,
		.total = hdarr_len(sim->world->ents)
	};

	struct nearest_applicable_ent_iter_ctx naeictx = {
		.origin = origin,
		.pred = pred,
		.ctx = usr_ctx,
	};

	while ((e = nearest_applicable_ent(sim, &naeictx))
	       && ctx.found < ctx.needed
	       && ctx.checked < ctx.total) {
		if (astar(pg, &e->pos, &ctx, ascb) == rs_fail) {
			break;
		}
	}

	return ctx.found;
}
