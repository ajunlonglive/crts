#ifndef SERVER_SIM_GEN_TERRAIN_H
#define SERVER_SIM_GEN_TERRAIN_H

#include "shared/math/linalg.h"
#include "shared/math/triangle.h"
#include "shared/sim/chunk.h"

struct terrain_vertex {
	const struct pointf *p;
	const struct tg_edge *faultedge;
	float elev;
	uint8_t fault;
	uint32_t filled;
	vec4 norm;
};

/* put all floats at the top */
struct terrain_pixel {
	float elev, watershed;

	float x, y;

	enum tile t;
	bool stream;
	vec4 norm;
};

struct terragen_opts {
	/* mesh */
	uint32_t _mesh;
	float radius;
	uint32_t height;
	uint32_t points;
	uint32_t seed;
	uint32_t width;

	/* faults */
	uint32_t _faults;
	float fault_boost_decay;
	float fault_max_ang;
	float fault_radius_pct_extent;
	uint32_t fault_max_len;
	uint32_t fault_mtn_mod;
	uint32_t fault_valley_chance;
	uint32_t fault_valley_max;
	uint32_t fault_valley_min;
	uint32_t fault_valley_mod;
	uint32_t faults;

	/* erosion */
	uint32_t _erosion;
	float deposition_rate;
	float raindrop_friction;
	float raindrop_speed;
	float erosion_rate;
	uint32_t raindrop_max_iterations;
	uint32_t raindrops;

	/* noise */
	uint32_t _noise;
	float final_noise_amp;
	uint32_t final_noise_octs;
	float final_noise_freq;
	float final_noise_lacu;

	/* write tiles */
	uint32_t _write_tiles;
	uint32_t upscale;
};

struct terrain {
	float height, width;
	float max_watershed;
	uint8_t faults;
	struct hdarr *tdat;
	struct darr *fault_points;
	struct pointf mid;
	float radius;
	struct terrain_pixel *heightmap;
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
	struct terragen_opts opts;
	enum terragen_step step;

	struct {
		bool tdat, trigraph;
	} init;
};

void terragen_init(struct terragen_ctx *ctx, const struct terragen_opts *opts);
void terragen(struct terragen_ctx *ctx, struct chunks *chunks);

struct terrain_pixel *get_terrain_pix(struct terragen_ctx *ctx, uint32_t x, uint32_t y);
#endif
