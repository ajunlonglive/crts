#include "posix.h"

#include <math.h>

#include "shared/math/rand.h"
#include "shared/types/darr.h"
#include "terragen/gen/faults.h"
#include "terragen/gen/gen.h"
#include "shared/util/log.h"

static void
gen_fault(struct terragen_ctx *ctx)
{
	bool first_pass = true;
	const uint8_t fault_id = ++ctx->terra.faults;
	const struct tg_edge *e = darr_get(hdarr_darr(ctx->tg.edges),
		rand_uniform(hdarr_len(ctx->tg.edges))), *oe = e;
	struct tg_tri *t;
	const struct pointf *p;

	/* float boost = rand_uniform(40) - 20; */
	float boost, oboost = rand_chance(ctx->opts.fault_valley_chance)
		? (ctx->opts.fault_valley_max - rand_uniform(ctx->opts.fault_valley_mod))
		: (rand_uniform(ctx->opts.fault_mtn_mod) + ctx->opts.fault_valley_min);

gen_fault_pass:
	boost = oboost;
	p = first_pass ? oe->a : oe->b;

	uint32_t i = 0;
	while (++i < ctx->opts.fault_max_len
	       && fsqdist(p, &ctx->terra.mid) < ctx->terra.radius * ctx->opts.fault_radius_pct_extent) {
		float angle = 0.0, nextang;

		struct terrain_vertex *tdat[] = {
			hdarr_get(ctx->terra.tdat, e->a),
			hdarr_get(ctx->terra.tdat, e->b)
		};

		if (p == e->a) {
			if (tdat[1]->fault) {
				break;
			}
			p = e->b;
		} else {
			if (tdat[0]->fault) {
				break;
			}
			p = e->a;
		}

		darr_push(ctx->terra.fault_points, p);

		if (!tdat[0]->fault) {
			tdat[0]->fault = fault_id;
			tdat[0]->filled = fault_id;
			tdat[0]->elev += boost;
			tdat[0]->faultedge = e;
		}

		if (!tdat[1]->fault) {
			tdat[1]->fault = fault_id;
			tdat[1]->filled = fault_id;
			tdat[1]->elev += boost;
			tdat[1]->faultedge = e;
		}

		t = hdarr_get(ctx->tg.tris, e->adja);
		while (angle < PI) {
			nextang = tg_point_angle(t, p);

			if (angle + nextang > ctx->opts.fault_max_ang && angle > 0.0) {
				break;
			}

			angle += nextang;

			const struct tg_edge *n = next_edge(&ctx->tg, t, e, p);

			e = n;
			if (tg_tris_eql(t, e->adja) && e->adjb[0]) {
				t = hdarr_get(ctx->tg.tris, e->adjb);
			} else {
				t = hdarr_get(ctx->tg.tris, e->adja);
			}
		}

		boost -= boost > 0 ? 1.0 : -1.0;
	}

	if (first_pass) {
		first_pass = false;
		goto gen_fault_pass;
	}
}

void
tg_gen_faults(struct terragen_ctx *ctx)
{
	rand_set_seed(ctx->opts.seed);

	L("generating %d faults", ctx->opts.faults);
	uint32_t i;
	for (i = 0; i < ctx->opts.faults; ++i) {
		gen_fault(ctx);
	}
}

struct fill_plates_recursor_ctx {
	struct terragen_ctx *ctx;
	const struct pointf *start;
	float closest;
	float boost;
	uint32_t id;
	float boost_decay;
};

static void
fill_plates_recursor(const struct pointf *p, const struct tg_edge *e, void *_ctx)
{
	const struct fill_plates_recursor_ctx *ctx = _ctx;
	struct terrain_vertex *td;
	float dist;

	if ((dist = fsqdist(p, ctx->start)) < ctx->closest) {
		return;
	} else if ((td = hdarr_get(ctx->ctx->terra.tdat, p))->filled & ctx->id || td->fault) {
		return;
	}

	/* td->elev += ctx->boost * (10 / sqrt(dist)); */
	td->elev += ctx->boost;
	td->filled |= ctx->id;

	if (fabs(ctx->boost) < 1) {
		return;
	}

	struct fill_plates_recursor_ctx new_ctx = *ctx;
	new_ctx.closest = dist;
	new_ctx.boost = ctx->boost * ctx->boost_decay;

	tg_for_each_adjacent_point(&ctx->ctx->tg, p, e, &new_ctx, fill_plates_recursor);
}

void
tg_fill_plates(struct terragen_ctx *ctx)
{
	uint32_t i;
	for (i = 0; i < darr_len(ctx->terra.fault_points); ++i) {
		struct terrain_vertex *td = hdarr_get(ctx->terra.tdat,
			darr_get(ctx->terra.fault_points, i));

		/* TODO investigate why this occurs */
		if (!td->fault) {
			continue;
		}

		struct fill_plates_recursor_ctx rctx = {
			.ctx = ctx,
			.start = td->p,
			.closest = 0,
			.boost = td->elev,
			.id = td->fault,
			.boost_decay = ctx->opts.fault_boost_decay,
		};

		tg_for_each_adjacent_point(&ctx->tg, td->p, td->faultedge, &rctx, fill_plates_recursor);
	}
}
