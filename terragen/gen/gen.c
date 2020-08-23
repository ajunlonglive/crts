#include "posix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "shared/math/delaunay.h"
#include "shared/math/perlin.h"
#include "shared/math/rand.h"
#include "shared/sim/chunk.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"
#include "terragen/gen/erosion.h"
#include "terragen/gen/faults.h"
#include "terragen/gen/filters.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/rasterize.h"
#include "terragen/gen/write_tiles.h"

struct terrain_pixel *
get_terrain_pix(struct terragen_ctx *ctx, uint32_t x, uint32_t y)
{
	uint32_t index = (y * ctx->l) + x;
	assert(index < ctx->a);
	return &ctx->terra.heightmap[index];
}

static void
init_tdat(struct terragen_ctx *ctx)
{
	uint32_t i;

	for (i = 0; i < darr_len(ctx->tg.points); ++i) {
		struct pointf *p = darr_get(ctx->tg.points, i);
		struct terrain_vertex td = { .p = p, .elev = 0, .fault = false };
		hdarr_set(ctx->terra.tdat, p, &td);
	}

	ctx->init.tdat = true;
}

static void
gen_mesh(struct terragen_ctx *ctx)
{
	tg_scatter(&ctx->tg, ctx->l, ctx->l, ctx->opts[tg_points].u,
		ctx->opts[tg_radius].f);

	delaunay(&ctx->tg);
}

static enum terragen_step
determine_change(struct terragen_ctx *ctx, const terragen_opts opts)
{
	uint32_t i;
	/* const uint8_t *new = (uint8_t *)opts, *old = (uint8_t *)&ctx->opts, *off = NULL; */

	for (i = 0; i < tg_opt_count; ++i) {
		if (opts[i].u != ctx->opts[i].u) {
			break;
		}
	}

	if (i == tg_opt_count) {
		return tgs_done;
	} else if (i >= tg_upscale) {
		L("tiles changed");
		return tgs_tiles;
	} else if (i >= tg_noise) {
		L("noise changed");
		return tgs_post_noise;
	} else if (i >= tg_erosion_cycles) {
		L("erosion changed");
		return tgs_erosion;
	} else if (i >= tg_mountains) {
		L("faults changed");
		return tgs_faults;
	} else {
		L("mesh changed");
		return tgs_mesh;
	}
}

void
terragen_init(struct terragen_ctx *ctx, const terragen_opts opts)
{
	const struct pointf corner = { 0.0, 0.0 };

	ctx->step = ctx->step ? determine_change(ctx, opts) : tgs_init;
	memcpy(ctx->opts, opts, sizeof(terragen_opts));

	ctx->l = ctx->opts[tg_dim].u;
	ctx->a = ctx->l * ctx->l;

	size_t hms = ctx->a * sizeof(struct terrain_pixel);

	switch (ctx->step) {
	case tgs_init:
		ctx->terra.tdat = hdarr_init(2048, sizeof(struct pointf *),
			sizeof(struct terrain_vertex), NULL);
		ctx->terra.fault_points = darr_init(sizeof(struct pointf *));

		trigraph_init(&ctx->tg);
	/* FALLTHROUGH */
	case tgs_mesh:
		ctx->terra.mid = (struct pointf){ ctx->l * 0.5, ctx->l * 0.5 };
		ctx->terra.radius = fsqdist(&ctx->terra.mid, &corner) * opts[tg_radius].f;

		trigraph_clear(&ctx->tg);

		ctx->terra.heightmap = realloc(ctx->terra.heightmap, hms);
	/* FALLTHROUGH */
	case tgs_faults:
		ctx->init.tdat = false;
		hdarr_clear(ctx->terra.tdat);
		darr_clear(ctx->terra.fault_points);
	/* FALLTHROUGH */
	case tgs_raster:
		memset(ctx->terra.heightmap, 0, hms);
	/* FALLTHROUGH */
	case tgs_pre_blur:
	/* FALLTHROUGH */
	case tgs_pre_noise:
	/* FALLTHROUGH */
	case tgs_erosion:
	/* FALLTHROUGH */
	case tgs_post_blur:
	/* FALLTHROUGH */
	case tgs_post_noise:
	/* FALLTHROUGH */
	case tgs_tiles:
	/* FALLTHROUGH */
	case tgs_done:
		break;
	}

	if (!ctx->step) {
		ctx->step = tgs_mesh;
	}
}

void
terragen(struct terragen_ctx *ctx, struct chunks *chunks)
{
	switch (ctx->step) {
	case tgs_init:
		assert(false);
		break;
	case tgs_mesh:
		L("generating mesh");

		rand_set_seed(ctx->opts[tg_seed].u);

		gen_mesh(ctx);
	/* FALLTHROUGH */
	case tgs_faults:
		init_tdat(ctx);

		L("generating fault lines");
		rand_set_seed(ctx->opts[tg_seed].u);
		tg_gen_faults(ctx);
		L("filling tectonic plates");
		rand_set_seed(ctx->opts[tg_seed].u);
		tg_fill_plates(ctx);
	/* FALLTHROUGH */
	case tgs_raster:
		L("rasterizing terrain heightmap");
		tg_rasterize(ctx);

	/* FALLTHROUGH */
	case tgs_pre_blur:
		L("blurring elevations");
		tg_blur(ctx, 1.0, 7, 0, 1);
	/* FALLTHROUGH */
	case tgs_pre_noise:
	/* FALLTHROUGH */
	case tgs_erosion:
		L("simulating erosion");
		rand_set_seed(ctx->opts[tg_seed].u);
		tg_simulate_erosion(ctx);

		L("blurring moisture");
		tg_blur(ctx, 1.0, 7, 1, 1);

		L("tracing rivers");
		rand_set_seed(ctx->opts[tg_seed].u);
		tg_trace_rivers(ctx);
	/* FALLTHROUGH */
	case tgs_post_blur:
	/* FALLTHROUGH */
	case tgs_post_noise:
		L("adding noise");
		rand_set_seed(ctx->opts[tg_seed].u);
		perlin_noise_shuf();
		tg_add_noise(ctx);
	/* FALLTHROUGH */
	case tgs_tiles:
		if (chunks) {
			L("writing tiles");
			tg_write_tiles(ctx, chunks);
		}
	/* FALLTHROUGH */
	case tgs_done:
		break;
	}
}

/* free(ctx->terra.heightmap); */
/* hdarr_destroy(ctx->terra.tdat); */
/* darr_destroy(ctx->terra.fault_points); */
/* TODO: tgraph_destroy(&tg) */