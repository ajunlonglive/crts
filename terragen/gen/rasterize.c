#include "posix.h"

#include <assert.h>

#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/rasterize.h"

static void
rasterize_tri_cb(void *_ctx, float *vd_interp, size_t vd_len, int32_t x, int32_t y)
{
	struct terragen_ctx *ctx = _ctx;

	if (x < 0 || x >= (int32_t)ctx->l || y < 0 || y >= (int32_t)ctx->l) {
		return;
	}

	struct terrain_pixel *tp = get_terrain_pix(ctx, x, y);

	tp->initial_elev = vd_interp[0];

	tp->norm[0] = vd_interp[1];
	tp->norm[1] = vd_interp[2];
	tp->norm[2] = vd_interp[3];

	tp->x = x;
	tp->y = y;

	tp->filled = true;
}

void
tg_rasterize(struct terragen_ctx *ctx)
{
	uint32_t i;

	/* set initial elev */
	for (i = 0; i < ctx->a; ++i) {
		ctx->terra.heightmap[i].initial_elev = -5;
		ctx->terra.heightmap[i].filled = true;
	}

	for (i = 0; i < hdarr_len(&ctx->tg.tris); ++i) {
		struct tg_tri *t = darr_get(&ctx->tg.tris.darr, i);

		struct terrain_vertex *tdat[] = {
			hdarr_get(&ctx->terra.tdat, t->a),
			hdarr_get(&ctx->terra.tdat, t->b),
			hdarr_get(&ctx->terra.tdat, t->c)
		};

		assert(tdat[0] && tdat[1] && tdat[2]);

		vec4 points3d[3] = {
			{ t->a->x, tdat[0]->elev, t->a->y, },
			{ t->c->x, tdat[2]->elev, t->c->y, },
			{ t->b->x, tdat[1]->elev, t->b->y, },
		};

		vec4 norm;
		calc_normal(points3d[0], points3d[1], points3d[2], norm);

		vec_add(tdat[0]->norm, norm);
		vec_add(tdat[1]->norm, norm);
		vec_add(tdat[2]->norm, norm);
	}

	for (i = 0; i < hdarr_len(&ctx->terra.tdat); ++i) {
		struct terrain_vertex *tdat = darr_get(&ctx->terra.tdat.darr, i);

		vec_normalize(tdat->norm);
	}

	for (i = 0; i < hdarr_len(&ctx->tg.tris); ++i) {
		struct tg_tri *t = darr_get(&ctx->tg.tris.darr, i);

		struct terrain_vertex *tdat[] = {
			hdarr_get(&ctx->terra.tdat, t->a),
			hdarr_get(&ctx->terra.tdat, t->b),
			hdarr_get(&ctx->terra.tdat, t->c)
		};

		assert(tdat[0] && tdat[1] && tdat[2]);

		float vertex_data[][3] = {
			{ tdat[0]->elev, tdat[1]->elev, tdat[2]->elev, },
			{ tdat[0]->norm[0], tdat[1]->norm[0], tdat[2]->norm[0], },
			{ tdat[0]->norm[1], tdat[1]->norm[1], tdat[2]->norm[1], },
			{ tdat[0]->norm[2], tdat[1]->norm[2], tdat[2]->norm[2], },
		};

		rasterize_tri(t, ctx, vertex_data, 4, rasterize_tri_cb);
	}
}
