#include "posix.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include "shared/math/rand.h"
#include "shared/util/log.h"
#include "terragen/gen/erosion.h"
#include "terragen/gen/gen.h"

#define VERTICAL 1.5707963268f

#define RAIN_PROB 0.001f
#define RAINDROP 45.0f

#define EVAPORATION 1.0f

#define Kc 0.002f /* capacity */
#define Ks 0.001f /* solubility */
#define Kd 0.01f /* deposition */

#define g 9.8f
#define A 0.01f
#define len 0.02f
#define lx 0.02f
#define ly 0.02f
#define DT 0.002f

static void
erosion_setup(struct terragen_ctx *ctx)
{
	uint32_t i;

	for (i = 0; i < ctx->a; ++i) {
		ctx->terra.heightmap[i].elev = ctx->terra.heightmap[i].initial_elev;
		memset(&ctx->terra.heightmap[i].e, 0, sizeof(ctx->terra.heightmap[i].e));
		if (ctx->terra.heightmap[i].elev > 0) {

			ctx->terra.heightmap[i].e.d = 0.01f;
		}

		ctx->terra.heightmap[i].tilt = VERTICAL;
	}
}

static void
inc_water(struct terragen_ctx *ctx)
{
	/* uint32_t i, a = ctx->opts.height * ctx->opts.width; */

	/* for (i = 0; i < a; ++i) { */

	/* if (RAIN_PROB > drand48()) { */
	/* 	ctx->terra.heightmap[i].e.d += RAINDROP; */
	/* } */

	/* if (drand48() < 0.8) { */
	/* 	return; */
	/* } */

	float x = drand48() * (float)ctx->l,

	      y = drand48() * (float)ctx->l;

	/* float x = 128, y = 128; */
	/* TODO, does nothing */
	get_terrain_pix(ctx, x, y)->e.d += 0.0;

	/* if (ctx->terra.heightmap[i].e.r) { */
	/* 	ctx->terra.heightmap[i].e.d += 1.0; */
	/* } */
	/* } */
}

static bool
dec_water(struct terragen_ctx *ctx)
{
	uint32_t i;
	bool found = false;

	for (i = 0; i < ctx->a; ++i) {
#if 0
		if (ctx->terra.heightmap[i].elev < 0) {
			ctx->terra.heightmap[i].e.d = 0;
			continue;
		} else
#endif
		if (ctx->terra.heightmap[i].e.d == 0.0f) {
			continue;
		}

		found = true;

		if ((ctx->terra.heightmap[i].e.d *=
			     (1 - EVAPORATION * DT)) < 0.001) {
			ctx->terra.heightmap[i].e.d = 0.0f;
			ctx->terra.heightmap[i].elev +=
				ctx->terra.heightmap[i].e.s;
			ctx->terra.heightmap[i].e.s1 =
				ctx->terra.heightmap[i].e.s = 0;
		}
	}

	return found;
}

static void
calc_flux(struct terrain_pixel *p, float *flux, const struct terrain_pixel *n)
{
	assert(p);

	if (!n) {
		*flux = 0;
		return;
#if 0
	} else if (p->elev < 0) {
		*flux = 0;
		return;
#endif
	}

	*flux += DT * A * (g * (p->elev + p->e.d - (n->elev + n->e.d))) / len;

	if (*flux < 0) {
		*flux = 0;
	}
}

static void
determine_flux(struct terragen_ctx *ctx)
{
	uint32_t x, y;

	const struct terrain_pixel *nbr[4];
	struct terrain_pixel *cur;

	for (x = 0; x < ctx->l; ++x) {
		for (y = 0; y < ctx->l; ++y) {
			cur = get_terrain_pix(ctx, x, y);
			get_neighbours(ctx, x, y, nbr);

			calc_flux(cur, &cur->e.f[L], nbr[L]);
			calc_flux(cur, &cur->e.f[R], nbr[R]);
			calc_flux(cur, &cur->e.f[T], nbr[T]);
			calc_flux(cur, &cur->e.f[B], nbr[B]);

			float K = cur->e.d * len / ((cur->e.f[L] + cur->e.f[R]
						     + cur->e.f[T] + cur->e.f[B])
						    * DT);
			if (K < 1) {
				cur->e.f[L] *= K;
				cur->e.f[R] *= K;
				cur->e.f[T] *= K;
				cur->e.f[B] *= K;
			}
		}
	}
}

static void
update_surface(struct terragen_ctx *ctx)
{
	uint32_t x, y;

	const struct terrain_pixel *nbr[4];
	struct terrain_pixel *cur;

	for (x = 0; x < ctx->l; ++x) {
		for (y = 0; y < ctx->l; ++y) {
			cur = get_terrain_pix(ctx, x, y);
			get_neighbours(ctx, x, y, nbr);

			float fin[4] = {
				nbr[L] ? nbr[L]->e.f[R] : 0,
				nbr[R] ? nbr[R]->e.f[L] : 0,
				nbr[T] ? nbr[T]->e.f[B] : 0,
				nbr[B] ? nbr[B]->e.f[T] : 0,
			};

			float sfin = fin[L] + fin[R] + fin[T] + fin[B],
			      sfout = cur->e.f[L] + cur->e.f[R] + cur->e.f[T] + cur->e.f[B];

			float d2 = cur->e.d + (DT * (sfin - sfout)) / len;

			/* if (fin[0] > 0 || fin[1] > 0 || fin[2] > 0 || fin[3] > 0) { */
			/* 	L("fin: %f, %f, %f, %f", fin[0], fin[1], fin[2], fin[3]); */
			/* } */
			float den = len * (cur->e.d + d2) * 0.5;
			float dwx = ((fin[L] - cur->e.f[L]) + (cur->e.f[R] - fin[R])) * 0.5;
			cur->e.v[0] = den != 0.0f ? dwx / den : 0.0f;

			float dwy = ((fin[T] - cur->e.f[T]) + (cur->e.f[B] - fin[B])) * 0.5;
			cur->e.v[1] = den != 0.0f ? dwy / den : 0.0f;

			cur->e.d = d2;

			float vel_mag = cur->e.v[0] * cur->e.v[0] + cur->e.v[1] * cur->e.v[1];

			if (vel_mag != 0) {
				/* L("%f, %f, %f", dwx, dwy, den); */
				/* L("%f, %f", cur->e.v[0], cur->e.v[1]); */

				float elev[4] = {
					nbr[L] ? nbr[L]->elev : cur->elev,
					nbr[R] ? nbr[R]->elev : cur->elev,
					nbr[T] ? nbr[T]->elev : cur->elev,
					nbr[B] ? nbr[B]->elev : cur->elev,
				};

				calc_heightmap_norm(elev,  cur->norm);

				/* L("%f, %f -> %f", cur->e.v[0], cur->e.v[1], vel_mag); */
				cur->tilt = PI * 0.5 - acos(fabs(cur->norm[2]));
				cur->e.C = Kc * sin(cur->tilt) * vel_mag;

				/* L("%f * sin(%2.2f) * %f = %f", Kc, R2D(tilt), vel_mag, cur->e.C); */
			}

			assert(cur->e.C >= 0.0f);
		}
	}
}

static void
erosion_depositon(struct terragen_ctx *ctx)
{
	uint32_t x, y;

	struct terrain_pixel *cur;

	for (x = 0; x < ctx->l; ++x) {
		for (y = 0; y < ctx->l; ++y) {
			cur = get_terrain_pix(ctx, x, y);

			if (cur->e.C > cur->e.s) {
				float erosion = Ks * (cur->e.C - cur->e.s);
				cur->elev -= erosion;
				cur->e.s += erosion;
			} else if (cur->e.C > 0.00001) {
				float deposit = Kd * (cur->e.s - cur->e.C);
				cur->elev += deposit;
				cur->e.s -= deposit;
			}
		}
	}
}

static float
try_get_terrain_pix_sediment(struct terragen_ctx *ctx, float x, float y)
{
	if (x >= 0 && x < ctx->l && y >= 0 && y < ctx->l) {
		return get_terrain_pix(ctx, x, y)->e.s;
	} else {
		return 0.0f;
	}
}

static void
sediment_transport(struct terragen_ctx *ctx)
{
	uint32_t x, y;

	struct terrain_pixel *cur;

	for (x = 0; x < ctx->l; ++x) {
		for (y = 0; y < ctx->l; ++y) {
			cur = get_terrain_pix(ctx, x, y);

			float px = x - cur->e.v[0] * DT,
			      py = y - cur->e.v[0] * DT;

			float nbr[4] = {
				try_get_terrain_pix_sediment(ctx, floorf(px), floorf(py)),
				try_get_terrain_pix_sediment(ctx, ceilf(px),  floorf(py)),
				try_get_terrain_pix_sediment(ctx, floorf(px), ceilf(py)),
				try_get_terrain_pix_sediment(ctx, ceilf(px),  ceilf(py)),
			};

			cur->e.s1 = nearest_neighbour(nbr[0], nbr[1], nbr[2], nbr[3], px, py);
		}
	}

	for (x = 0; x < ctx->l; ++x) {
		for (y = 0; y < ctx->l; ++y) {
			cur = get_terrain_pix(ctx, x, y);

			cur->e.s = cur->e.s1;
		}
	}

}

#define PROGRESS_STEPS 20

void
tg_simulate_erosion(struct terragen_ctx *ctx)
{
	erosion_setup(ctx);

	uint32_t i = 1;
	for (i = 0; i < ctx->opts[tg_erosion_cycles].u; ++i) {
		inc_water(ctx);
		determine_flux(ctx);
		update_surface(ctx);
		erosion_depositon(ctx);
		sediment_transport(ctx);
		if (!dec_water(ctx)) {
			break;
		}

		ctx->erosion_progress = i;
		if (ctx->opts[tg_erosion_cycles].u > PROGRESS_STEPS) {
			if (!(i % (ctx->opts[tg_erosion_cycles].u / PROGRESS_STEPS))) {
				L("%0.1f%% done", (float)i * 100.0f / (float)ctx->opts[tg_erosion_cycles].u);
			}
		}
	}

	for (i = 0; i < ctx->a; ++i) {
		ctx->terra.heightmap[i].e.d = 0;
	}
}
