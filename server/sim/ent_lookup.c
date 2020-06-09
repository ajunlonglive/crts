#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <assert.h>

#include "shared/sim/ent_buckets.h"
#include "server/sim/ent_lookup.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"
#include "shared/util/log.h"

struct nearest_applicable_ent_iter_ctx {
	struct ent *ret;
	const struct point *origin;
	ent_lookup_pred pred;
	void *ctx;
	uint32_t min_dist;
	struct hash *checked;
};

static enum iteration_result
nearest_applicable_ent_iter(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct nearest_applicable_ent_iter_ctx *ctx = _ctx;
	uint32_t dist;

	if (!(hash_get(ctx->checked, &e->id))
	    && ctx->pred(e, ctx->ctx)
	    && (dist = square_dist(&e->pos, ctx->origin)) < ctx->min_dist) {
		ctx->min_dist = dist;
		ctx->ret = e;
	}

	return ir_cont;
}

static struct ent *
nearest_applicable_ent(struct simulation *sim,
	struct nearest_applicable_ent_iter_ctx *ctx)
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
	struct hash *checked_hash;
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


	if (!(hash_get(ctx->checked_hash, &e->id))
	    && ctx->pred(e, ctx->ctx)) {
		ctx->cb(e, ctx->ctx);

		if (++ctx->found >= ctx->needed) {
			return ir_done;
		}

		L("checking ent %d @ (%d, %d)", e->id, e->pos.x, e->pos.y);
		hash_set(ctx->checked_hash, &e->id, 1);

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

enum result
ent_lookup(struct simulation *sim, struct ent_lookup_ctx *elctx)
{
	struct ent *e;
	enum result r;

	assert(elctx->init);

	struct ascb_ctx ascbctx = {
		.sim = sim,
		.pred = elctx->pred,
		.cb = elctx->cb,
		.ctx = elctx->usr_ctx,
		.found = elctx->found,
		.needed = elctx->needed,
		.checked_hash = elctx->checked,
		.checked = 0,
		.total = hdarr_len(sim->world->ents)
	};

	struct pgraph pg;
	pgraph_init(&pg, sim->world->chunks);

	struct nearest_applicable_ent_iter_ctx naeictx = {
		.origin = elctx->origin,
		.pred = elctx->pred,
		.ctx = elctx->usr_ctx,
		.checked = elctx->checked,
	};

	L("looking up %d ents around (%d, %d)", elctx->needed, elctx->origin->x,
		elctx->origin->y);

	while ((e = nearest_applicable_ent(sim, &naeictx))
	       && ascbctx.found < ascbctx.needed
	       && ascbctx.checked < ascbctx.total) {

		switch (r = astar(elctx->pg, &e->pos, &ascbctx, ascb,
			ASTAR_DEF_RADIUS)) {
		case rs_fail:
			elctx->found = ascbctx.found;
			r = rs_fail;
			goto free_return;
		case rs_done:
			continue;
		case rs_cont:
			r =  rs_cont;
			goto free_return;
		}
	}

	elctx->found = ascbctx.found;

	if (ascbctx.found < ascbctx.needed
	    && ascbctx.checked >= ascbctx.total) {
		r = rs_fail;
	} else {
		r = rs_done;
	}

free_return:
	pgraph_destroy(&pg);
	return r;
}

void
ent_lookup_reset(struct ent_lookup_ctx *elctx)
{
	hash_clear(elctx->checked);
	elctx->found = 0;
	elctx->needed = 0;
	elctx->init = false;
}

void
ent_lookup_setup(struct ent_lookup_ctx *elctx)
{
	elctx->checked = hash_init(2048, 1, sizeof(ent_id_t));
}


void
ent_lookup_teardown(struct ent_lookup_ctx *elctx)
{
	hash_destroy(elctx->checked);
}
