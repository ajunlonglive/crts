#include "posix.h"

#include <math.h>

#include "shared/math/rand.h"
#include "shared/types/darr.h"
#include "terragen/gen/faults.h"
#include "terragen/gen/gen.h"
#include "shared/util/log.h"

static void
gen_fault(struct terragen_ctx *ctx, float mul)
{
	bool first_pass = true;
	const uint8_t fault_id = ++ctx->terra.faults;
	const struct tg_edge *e = darr_get(hdarr_darr(ctx->tg.edges),
		rand_uniform(hdarr_len(ctx->tg.edges))), *oe = e;
	struct tg_tri *t;
	const struct pointf *p;

	/* float boost = rand_uniform(40) - 20; */
	float boost, oboost = mul * drand48() * (ctx->opts[tg_height_mod].f) + 10;

gen_fault_pass:
	boost = oboost;
	p = first_pass ? oe->a : oe->b;

	uint32_t i = 0;
	while (++i < 1000
	       && fsqdist(p, &ctx->terra.mid) < ctx->terra.radius * 100) {
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
			tdat[0]->elev = tdat[0]->boost += boost;
			tdat[0]->faultedge = e;
		}

		if (!tdat[1]->fault) {
			tdat[1]->fault = fault_id;
			tdat[1]->filled = fault_id;
			tdat[0]->elev = tdat[1]->boost += boost;
			tdat[1]->faultedge = e;
		}

		t = hdarr_get(ctx->tg.tris, e->adja);
		while (angle < PI) {
			nextang = tg_point_angle(t, p);

			if (angle + nextang > PI * 2.0f * ctx->opts[tg_fault_curve].f
			    && angle > 0.0) {
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

		/* boost -= boost > 0 ? 1.0 : -1.0; */
	}

	if (first_pass) {
		first_pass = false;
		goto gen_fault_pass;
	}
}

void
tg_gen_faults(struct terragen_ctx *ctx)
{
	uint32_t i;
	for (i = 0; i < ctx->opts[tg_mountains].u; ++i) {
		gen_fault(ctx, 1);
	}

	for (i = 0; i < ctx->opts[tg_valleys].u; ++i) {
		gen_fault(ctx, -1);
	}
}

void
tg_fill_plates(struct terragen_ctx *ctx)
{
	float dist;
	uint32_t i, j, len = hdarr_len(ctx->terra.tdat);
	struct terrain_vertex *td, *cur;

	float d = ctx->opts[tg_fault_radius].f * ctx->opts[tg_fault_radius].f;

	for (i = 0; i < len; ++i) {
		if ((cur = darr_get(hdarr_darr(ctx->terra.tdat), i))->fault) {
			/* continue; */
		}

		for (j = 0; j < len; ++j) {
			if (j == i
			    || !(td = darr_get(hdarr_darr(ctx->terra.tdat), j))->fault
			    || (dist = fsqdist(cur->p, td->p)) > d) {
				continue;
			}

			cur->elev += td->boost * (1.0f - (dist / d));
		}
	}
}
