#include "posix.h"

#include <assert.h>
#include <math.h>

#include "shared/math/rand.h"
#include "terragen/gen/erosion.h"
#include "terragen/gen/gen.h"

static void
trace_raindrop(struct terragen_ctx *ctx, float x, float y)
{
	float sediment = 0;
	float vx = 0;
	float vy = 0;

	uint32_t i, xi = roundf(x), yi = roundf(y);
	struct terrain_pixel *next = NULL,
			     *cur = get_terrain_pix(ctx, xi, yi);

	for (i = 0; i < ctx->opts.raindrop_max_iterations; ++i) {
		vx = ctx->opts.raindrop_friction * vx
		     + cur->norm[0] * ctx->opts.raindrop_speed;
		vy = ctx->opts.raindrop_friction * vy
		     + cur->norm[2] * ctx->opts.raindrop_speed;

		if (fabs(vx) < 0.02 && fabs(vy) < 0.02) {
			if (rand_chance(2)) {
				vx = rand_chance(2) ? 0.2 : -0.2;
			} else {
				vy = rand_chance(2) ? 0.2 : -0.2;
			}
		}

		do {
			x += vx;
			y += vy;
			xi = roundf(x); yi = roundf(y);

			if (xi < 1 || xi >= ctx->terra.width - 1
			    || yi < 1 || yi >= ctx->terra.height - 1) {
				return;
			}
		} while ((next = get_terrain_pix(ctx, xi, yi)) == cur);

		if (cur->watershed > ctx->terra.max_watershed) {
			/* L("lots of water at %f, %f", x, y); */
			goto move_raindrop;
		}

		assert(next);

		if (next->norm[1] == 1 || next->elev < -2) {
			break;
		}
		float deposit = sediment * ctx->opts.deposition_rate * next->norm[1];
		float erosion = ctx->opts.erosion_rate * (1 - next->norm[1]);

		if (next->elev < cur->elev) {
			vx = vy = 0;

			sediment -= deposit;
			cur->elev += deposit;

			if (sediment < 0) {
				break;
			}
		} else {
			sediment += erosion - deposit;
			/* L("eroding by %f", deposit - erosion); */
			cur->elev += deposit - erosion;
		}

		++cur->watershed;
move_raindrop:
		cur = next;
	}
}

static void
trace_river(struct terragen_ctx *ctx, struct terrain_pixel *cur)
{
	float vx = 0;
	float vy = 0;

	struct terrain_pixel *next = NULL;

	float x = cur->x, y = cur->y;

	uint32_t i;
	int32_t xi, yi;
	for (i = 0; i < 100; ++i) {
		vx = ctx->opts.raindrop_friction * vx
		     + cur->norm[0] * ctx->opts.raindrop_speed;
		vy = ctx->opts.raindrop_friction * vy
		     + cur->norm[2] * ctx->opts.raindrop_speed;

		if (fabs(vx) < 0.02 && fabs(vy) < 0.02) {
			if (rand_chance(2)) {
				vx = rand_chance(2) ? 0.2 : -0.2;
			} else {
				vy = rand_chance(2) ? 0.2 : -0.2;
			}
		}

		do {
			x += vx;
			y += vy;
			xi = roundf(x); yi = roundf(y);

			if (xi < 1 || xi >= ctx->terra.width - 1
			    || yi < 1 || yi >= ctx->terra.height - 1) {
				return;
			}
		} while ((next = get_terrain_pix(ctx, xi, yi)) == cur);

		assert(next);

		if (next->elev < -4) {
			break;
		}

		if (next->elev > cur->elev) {
			next->elev = cur->elev;
		}

		/* float deposit = sediment * terra->opts.deposition_rate * tp->norm[1]; */
		/* float erosion = terra->opts.erosion_rate * (1 - tp->norm[1]); */
		/* float height_diff = deposit - erosion; */

		cur->stream = true;

		cur = next;

		/* ++ptp->watershed; */

		/* ptp = tp; */
	}
}

void
tg_simulate_erosion(struct terragen_ctx *ctx)
{
	uint32_t rx, ry;

	for (ry = 0; ry < ctx->terra.height; ++ry) {
		for (rx = 0; rx < ctx->terra.width; ++rx) {
			/* uint32_t i; */
			/* for (i = 0; i < terra->opts.raindrops; ++i) { */
			trace_raindrop(ctx,
				rand_uniform(ctx->terra.width),
				rand_uniform(ctx->terra.height));
			/* 	float pct = (float)i / terra->opts.raindrops * 100.0; */
			/* 	if (i % 100000 == 0) { */
			/* 		L("%f%% done", pct); */
			/* 	} */
			/* } */
		}
	}
}

void
tg_trace_rivers(struct terragen_ctx *ctx)
{
	uint32_t rx, ry;

	for (ry = 0; ry < ctx->terra.height; ++ry) {
		for (rx = 0; rx < ctx->terra.width; ++rx) {
			struct terrain_pixel *tp = get_terrain_pix(ctx, rx, ry);

			tp->watershed /= ctx->terra.max_watershed;

			if (tp->watershed > 0.4) {
				trace_river(ctx, tp);
			}
		}
	}
}

