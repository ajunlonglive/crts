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

	if (tp->tilt > 0.8) {
		ck->tiles[rx][ry] = tile_rock;
	} else if (tp->elev < 0) {
		ck->tiles[rx][ry] = tile_sea;
	} else if (tp->elev < 0.5) {
		tp->elev -= 0.5;
		ck->tiles[rx][ry] = tile_sea;
	} else if (tp->elev < 3) {
		if (moisture < 0.5) {
			ck->tiles[rx][ry] = tile_coast;
		} else if (moisture < 0.6) {
			ck->tiles[rx][ry] = tile_old_tree;
		} else {
			ck->tiles[rx][ry] = tile_tree;
		}
	} else if (tp->elev < 30) {
		if (moisture < 0.7 || tp->tilt > 0.7) {
			ck->tiles[rx][ry] = tile_plain;
		} else if (moisture < 0.8) {
			ck->tiles[rx][ry] = tile_old_tree;
		} else {
			ck->tiles[rx][ry] = tile_tree;
		}
	} else if (tp->elev < 50) {
		ck->tiles[rx][ry] = tile_rock;
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

			tp.tilt = nearest_neighbour(
				nbr[0] ? nbr[0]->tilt : 0,
				nbr[1] ? nbr[1]->tilt : 0,
				nbr[2] ? nbr[2]->tilt : 0,
				nbr[3] ? nbr[3]->tilt : 0,
				x, y);

			tp.x = q.x;
			tp.y = q.y;

			write_tile(ck, &tp, rx, ry);
		}
	}
}

#define VERTICAL 1.5707963268f
static void
normalize_tilts(struct terragen_ctx *ctx)
{
	uint32_t i;
	struct terrain_pixel *tp;

	for (i = 0; i < ctx->a; ++i) {
		tp = &ctx->terra.heightmap[i];
		if (tp->tilt > VERTICAL) {
			tp->tilt = 0.0f;
		} else {
			tp->tilt = 1.0f - (tp->tilt / VERTICAL);
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

	normalize_tilts(ctx);

	for (p.x = 0; p.x < dim; p.x += CHUNK_SIZE) {
		for (p.y = 0; p.y < dim; p.y += CHUNK_SIZE) {
			struct chunk *ck = get_chunk(chunks, &p);

			write_chunk(ctx, ck, r);
		}
	}
}
