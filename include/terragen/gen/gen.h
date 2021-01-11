#ifndef SERVER_SIM_GEN_TERRAIN_H
#define SERVER_SIM_GEN_TERRAIN_H

#include "shared/math/linalg.h"
#include "shared/math/triangle.h"
#include "shared/sim/chunk.h"
#include "terragen/gen/opts.h"

enum tile_sides { L, R, T, B };

struct terrain_vertex {
	const struct pointf *p;
	const struct tg_edge *faultedge;
	float elev, boost;
	uint8_t fault;
	uint32_t filled;
	vec4 norm;
};

/* put all floats at the top */
struct terrain_pixel {
	float initial_elev, elev;

	float x, y;

	enum tile t;
	bool stream;
	vec4 norm;
	float tilt;

	bool filled;

	struct {
		float d, s, s1, C;
		vec4 f;
		float v[2];
	} e;
};

struct terrain {
	struct hdarr tdat;
	struct darr fault_points;
	struct pointf mid;
	struct terrain_pixel *heightmap;
	uint8_t faults;
	float radius;
};

enum terragen_step {
	tgs_init,
	tgs_mesh,
	tgs_faults,
	tgs_raster,
	tgs_pre_blur,
	tgs_pre_noise,
	tgs_erosion,
	tgs_post_blur,
	tgs_post_noise,
	tgs_tiles,
	tgs_done,
};

struct terragen_ctx {
	struct trigraph tg;
	struct terrain terra;
	terragen_opts opts;
	enum terragen_step step;

	struct {
		bool tdat, trigraph;
	} init;

	uint32_t erosion_progress;
	uint32_t a, l;
};

void terragen_init(struct terragen_ctx *ctx, const terragen_opts opts);
void terragen_destroy(struct terragen_ctx *ctx);
void terragen(struct terragen_ctx *ctx, struct chunks *chunks);

struct terrain_pixel *get_terrain_pix(struct terragen_ctx *ctx, uint32_t x, uint32_t y);
void get_neighbours(struct terragen_ctx *ctx, float x, float y, const struct terrain_pixel *nbr[4]);
void get_nearest_neighbours(struct terragen_ctx *ctx, float x, float y,
	const struct terrain_pixel *nbr[4]);
void calc_heightmap_norm(float elev[4], vec4 norm);
#endif
