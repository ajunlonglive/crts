#include "posix.h"

#include <assert.h>
#include <math.h>

#include "shared/math/geom.h"
#include "shared/math/perlin.h"
#include "shared/util/log.h"
#include "terragen/gen/gen.h"
#include "terragen/gen/write_tiles.h"

static void
write_tile(struct chunk *ck, struct terrain_pixel *tp, uint32_t rx, uint32_t ry)
{
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

	ck->heights[rx][ry] = tp->elev;
}

static void
write_chunk(struct terragen_ctx *ctx, struct chunk *ck, float r)
{
	struct point q;
	uint32_t rx, ry;

	for (rx = 0; rx < CHUNK_SIZE; ++rx) {
		for (ry = 0; ry < CHUNK_SIZE; ++ry) {
			q.x = ck->pos.x + rx;
			q.y = ck->pos.y + ry;

			float x = q.x * r, y = q.y * r;

			const struct terrain_pixel *nbr[4] = { 0 };
			struct terrain_pixel tp = { 0 };

			get_nearest_neighbours(ctx, x, y, nbr);

			tp.elev = nearest_neighbour(
				nbr[0] ? nbr[0]->elev : 0,
				nbr[1] ? nbr[1]->elev : 0,
				nbr[2] ? nbr[2]->elev : 0,
				nbr[3] ? nbr[3]->elev : 0,
				x, y);

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
