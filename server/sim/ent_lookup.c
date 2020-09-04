#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <assert.h>

#include "shared/sim/ent_buckets.h"
#include "server/sim/ent_lookup.h"
#include "server/sim/pathfind/pathfind.h"
#include "server/sim/sim.h"
#include "shared/util/log.h"

struct ent_count_ctx {
	uint32_t count;
	void *usr_ctx;
	ent_lookup_pred pred;
};

static enum iteration_result
ent_count_iter(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct ent_count_ctx *ctx = _ctx;

	if (ctx->pred(e, ctx->usr_ctx)) {
		++ctx->count;
	}

	return ir_cont;
}

uint32_t
ent_count(struct hdarr *ents, void *ctx, ent_lookup_pred pred)
{
	struct ent_count_ctx elctx = { 0, ctx, pred };

	hdarr_for_each(ents, &elctx, ent_count_iter);

	return elctx.count;
}

struct nearest_applicable_ent_iter_ctx {
	struct ent *ret;
	struct ent_lookup_ctx *elctx;
	uint32_t min_dist;
	uint32_t radius_squared;
};

static enum iteration_result
nearest_applicable_ent_iter(void *_ctx, void *_e)
{
	struct ent *e = _e;
	struct nearest_applicable_ent_iter_ctx *ctx = _ctx;
	uint32_t dist;

	if (!(hash_get(ctx->elctx->checked_hash, &e->id))
	    && (dist = square_dist(&e->pos, ctx->elctx->origin)) < ctx->min_dist
	    && dist < ctx->radius_squared
	    && ctx->elctx->pred(e, ctx->elctx->usr_ctx)) {
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

static enum iteration_result
check_ents_at(void *_ctx, struct ent *e)
{
	struct ent_lookup_ctx *ctx = _ctx;

	if (!(hash_get(ctx->checked_hash, &e->id))
	    && ctx->pred(e, ctx->usr_ctx)) {
		ctx->cb(e, ctx->usr_ctx);

		if (++ctx->found >= ctx->needed) {
			return ir_done;
		}

		/* L("checking ent %d @ (%d, %d)", e->id, e->pos.x, e->pos.y); */
		hash_set(ctx->checked_hash, &e->id, 1);

		++ctx->checked;
	}

	return ir_cont;
}

static enum result
ascb(void *_ctx, const struct point *p)
{
	struct ent_lookup_ctx *ctx = _ctx;

	for_each_ent_at(&ctx->sim->eb, ctx->sim->world->ents, p,
		ctx, check_ents_at);

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

	/* TODO: have callers set this */
	elctx->radius = ASTAR_DEF_RADIUS;
	elctx->sim = sim;

	elctx->total = hdarr_len(sim->world->ents);

	struct nearest_applicable_ent_iter_ctx naeictx = {
		.elctx = elctx,
		.radius_squared = elctx->radius * elctx->radius,
	};

	while ((e = nearest_applicable_ent(sim, &naeictx))
	       && elctx->found < elctx->needed
	       && elctx->checked < elctx->total) {
		switch (r = astar(elctx->pg, &e->pos, elctx, ascb, elctx->radius)) {
		case rs_done:
			continue;
		case rs_cont:
		case rs_fail:
			return r;
		}
	}

	if (elctx->found < elctx->needed && elctx->checked >= elctx->total) {
		r = rs_fail;
	} else {
		r = rs_done;
	}

	return r;
}

void
ent_lookup_reset(struct ent_lookup_ctx *elctx)
{
	hash_clear(elctx->checked_hash);
	elctx->found = 0;
	elctx->needed = 0;
	elctx->init = false;
}

void
ent_lookup_setup(struct ent_lookup_ctx *elctx)
{
	elctx->checked_hash = hash_init(2048, 1, sizeof(ent_id_t));
}


void
ent_lookup_teardown(struct ent_lookup_ctx *elctx)
{
	hash_destroy(elctx->checked_hash);
}
