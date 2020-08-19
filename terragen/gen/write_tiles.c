#include "posix.h"

#include <assert.h>
#include <math.h>

#include "shared/math/perlin.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/write_tiles.h"

static void
write_tile(struct chunk *ck, struct terrain_pixel *tp, uint32_t rx, uint32_t ry)
{
	/* if (tp->stream > 0.3) { */
	/* 	ck->tiles[rx][ry] = tile_stream; */
	/* 	goto set_elev; */
	/* } */

	float moisture = (perlin_two(tp->x, tp->y, 1.0, 3, 0.33, 1.0) + 1.0) * 0.5;

	if (tp->elev < -5) {
		ck->tiles[rx][ry] = tile_deep_water;
	} else if (tp->elev < 0) {
		ck->tiles[rx][ry] = tile_water;
	} else if (tp->elev < 3) {
		if (moisture < 0.5) {
			ck->tiles[rx][ry] = tile_wetland;
		} else if (moisture < 0.6) {
			ck->tiles[rx][ry] = tile_wetland_forest_old;
		} else {
			ck->tiles[rx][ry] = tile_wetland_forest;
		}
	} else if (tp->elev < 30) {
		if (moisture < 0.7) {
			ck->tiles[rx][ry] = tile_plain;
		} else if (moisture < 0.8) {
			ck->tiles[rx][ry] = tile_forest_old;
		} else {
			ck->tiles[rx][ry] = tile_forest;
		}
	} else if (tp->elev < 40) {
		ck->tiles[rx][ry] = tile_mountain;
	} else {
		ck->tiles[rx][ry] = tile_peak;
	}

/* set_elev: */
	ck->heights[rx][ry] = tp->elev;
}

#define LERP(a, b, c, d) (a) * (1 - diffx) * (1 - diffy) + (b) * diffx * (1 - diffy) \
	+ (c) * diffy * (1 - diffx) + (d) * diffx * diffy

static void
write_chunk(struct terragen_ctx *ctx, struct chunk *ck, float r)
{
	struct point q;
	uint32_t rx, ry;

	for (rx = 0; rx < CHUNK_SIZE; ++rx) {
		for (ry = 0; ry < CHUNK_SIZE; ++ry) {
			q.x = ck->pos.x + rx;
			q.y = ck->pos.y + ry;

			float x = q.x * r, y = q.y * r,
			      diffx = x - floorf(x), diffy = y - floorf(y);
			uint32_t index = floorf(y) * ctx->l + floorf(x);

			struct terrain_pixel *nbr[4] = {
				&ctx->terra.heightmap[index],
				&ctx->terra.heightmap[index + 1],
				&ctx->terra.heightmap[index + ctx->l],
				&ctx->terra.heightmap[index + ctx->l + 1],
			}, tp;

			tp.elev = LERP(nbr[0]->elev,
				nbr[1]->elev,
				nbr[2]->elev,
				nbr[3]->elev);
			tp.stream = LERP((float)nbr[0]->stream,
				(float)nbr[1]->stream,
				(float)nbr[2]->stream,
				(float)nbr[3]->stream);

			tp.x = q.x;
			tp.y = q.y;

			write_tile(ck, &tp, rx, ry);
		}
	}
}

void
tg_write_tiles(struct terragen_ctx *ctx, struct chunks *chunks)
{
	struct point p = { 0, 0 };

	assert(ctx->l % CHUNK_SIZE == 0);

	int32_t dim = ctx->l * ctx->opts[tg_upscale].u;

	float r = 1.0 / (float)ctx->opts[tg_upscale].u;

	for (p.x = 0; p.x < dim; p.x += CHUNK_SIZE) {
		for (p.y = 0; p.y < dim; p.y += CHUNK_SIZE) {
			struct chunk *ck = get_chunk(chunks, &p);

			write_chunk(ctx, ck, r);
		}
	}
}
