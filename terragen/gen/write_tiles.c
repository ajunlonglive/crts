#include "posix.h"

#include <assert.h>
#include <math.h>

#include "terragen/gen/gen.h"
#include "terragen/gen/write_tiles.h"

static void
write_tile(struct chunk *ck, struct terrain_pixel *tp, uint32_t rx, uint32_t ry)
{
	if (tp->stream > 0.3) {
		ck->tiles[rx][ry] = tile_stream;
		goto set_elev;
	}

	if (tp->elev < -5) {
		ck->tiles[rx][ry] = tile_deep_water;
	} else if (tp->elev < 0) {
		ck->tiles[rx][ry] = tile_water;
	} else if (tp->elev < 3) {
		if (tp->watershed < 0.5) {
			ck->tiles[rx][ry] = tile_wetland;
		} else if (tp->watershed < 0.6) {
			ck->tiles[rx][ry] = tile_wetland_forest_old;
		} else {
			ck->tiles[rx][ry] = tile_wetland_forest;
		}
	} else if (tp->elev < 30) {
		if (tp->watershed < 0.7) {
			ck->tiles[rx][ry] = tile_plain;
		} else if (tp->watershed < 0.8) {
			ck->tiles[rx][ry] = tile_forest_old;
		} else {
			ck->tiles[rx][ry] = tile_forest;
		}
	} else if (tp->elev < 40) {
		ck->tiles[rx][ry] = tile_mountain;
	} else {
		ck->tiles[rx][ry] = tile_peak;
	}

set_elev:
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
			uint32_t index = floorf(y) * ctx->opts.width + floorf(x);

			struct terrain_pixel *nbr[4] = {
				&ctx->terra.heightmap[index],
				&ctx->terra.heightmap[index + 1],
				&ctx->terra.heightmap[index + ctx->opts.width],
				&ctx->terra.heightmap[index + ctx->opts.width + 1],
			}, tp;

			tp.elev = LERP(nbr[0]->elev,
				nbr[1]->elev,
				nbr[2]->elev,
				nbr[3]->elev);
			tp.watershed = LERP(nbr[0]->watershed,
				nbr[1]->watershed,
				nbr[2]->watershed,
				nbr[3]->watershed);
			tp.stream = LERP(nbr[0]->stream,
				nbr[1]->stream,
				nbr[2]->stream,
				nbr[3]->stream);

			write_tile(ck, &tp, rx, ry);
		}
	}
}

void
tg_write_tiles(struct terragen_ctx *ctx, struct chunks *chunks)
{
	struct point p = { 0, 0 };

	assert(ctx->opts.width % CHUNK_SIZE == 0);
	assert(ctx->opts.height % CHUNK_SIZE == 0);

	int32_t width = ctx->opts.width * ctx->opts.upscale,
		height = ctx->opts.height * ctx->opts.upscale;

	float r = 1.0 / (float)ctx->opts.upscale;

	for (p.x = 0; p.x < width; p.x += CHUNK_SIZE) {
		for (p.y = 0; p.y < height; p.y += CHUNK_SIZE) {
			struct chunk *ck = get_chunk(chunks, &p);

			write_chunk(ctx, ck, r);
		}
	}
}
