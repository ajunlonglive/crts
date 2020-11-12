#include "posix.h"

#ifndef CRTS_SERVER
#define CRTS_SERVER
#endif

#include <string.h>
#include <assert.h>

#include "server/sim/ent_lookup.h"
#include "server/sim/sim.h"
#include "shared/pathfind/pathfind.h"
#include "shared/sim/ent_buckets.h"
#include "shared/types/bheap.h"
#include "shared/util/log.h"
#include "tracy.h"

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
	struct simulation *sim;
	struct ent_lookup_ctx *elctx;
	const struct point *bucket;
	uint32_t min_dist;
};

struct bucketinfo {
	uint32_t dist;
	const struct point *p;
};

static enum iteration_result
add_bucket_to_heap(void *_ctx, const struct point *p)
{
	struct nearest_applicable_ent_iter_ctx *ctx = _ctx;

	struct bucketinfo bi = {
		.dist = square_dist(p, ctx->elctx->origin),
		.p = p
	};

	darr_push(ctx->elctx->bucketheap, &bi);

	return ir_cont;
}

static enum iteration_result
check_ents_in_bucket(void *_ctx, struct ent *e)
{
	TracyCZoneAutoS;
	struct nearest_applicable_ent_iter_ctx *ctx = _ctx;

	if (ctx->elctx->exclude && hash_get(ctx->elctx->exclude, &e->id)) {
		goto cont;
	} else if (!ctx->elctx->pred(e, ctx->elctx->usr_ctx)) {
		goto cont;
	}

	ctx->elctx->cb(e, ctx->elctx->usr_ctx);

	if (++ctx->elctx->found >= ctx->elctx->needed
	    || ++ctx->elctx->checked >= ctx->elctx->total) {
		TracyCZoneAutoE;
		return ir_done;
	}

cont:
	TracyCZoneAutoE;
	return ir_cont;
}

static void
check_ent_buckets(struct simulation *sim,
	struct nearest_applicable_ent_iter_ctx *ctx)
{
	struct bucketinfo *bi;

	while (darr_len(ctx->elctx->bucketheap)) {
		bi = bheap_peek(ctx->elctx->bucketheap);

		if (for_each_ent_in_bucket(&ctx->sim->eb, ctx->sim->world->ents,
			bi->p, ctx, check_ents_in_bucket)) {
			break;
		}

		bheap_pop(ctx->elctx->bucketheap);
	}
}

bool
ent_lookup(struct simulation *sim, struct ent_lookup_ctx *elctx)
{
	TracyCZoneAutoS;

	assert(elctx->init);

	elctx->sim = sim;

	elctx->total = hdarr_len(sim->world->ents);

	struct nearest_applicable_ent_iter_ctx naeictx = {
		.elctx = elctx,
		.sim = sim,
	};

	TracyCZoneN(tctx_buckets, "setup buckets", true);

	darr_clear(elctx->bucketheap);
	for_each_bucket(&sim->eb, &naeictx, add_bucket_to_heap);
	bheap_heapify(elctx->bucketheap);

	TracyCZoneEnd(tctx_buckets);

	check_ent_buckets(sim, &naeictx);

	if (elctx->found < elctx->needed && elctx->checked >= elctx->total) {
		TracyCZoneAutoE;
		return false;
	} else {
		TracyCZoneAutoE;
		return true;
	}
}

void
ent_lookup_reset(struct ent_lookup_ctx *elctx)
{
	elctx->found = 0;
	elctx->needed = 0;
	elctx->init = false;
}

void
ent_lookup_setup(struct ent_lookup_ctx *elctx)
{
	elctx->bucketheap = darr_init(sizeof(struct bucketinfo));
}

void
ent_lookup_teardown(struct ent_lookup_ctx *elctx)
{
	darr_destroy(elctx->bucketheap);
}
