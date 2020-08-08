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

struct worldgen_opts {
	uint32_t fault_max_len,
		 fault_valley_chance,
		 fault_valley_max,
		 fault_valley_mod,
		 fault_mtn_mod,
		 fault_valley_min,
		 raindrop_max_iterations,
		 final_noise_octs,
		 height,
		 width,
		 points,
		 faults,
		 raindrops,
		 upscale,
		 seed;
	float radius,
	      fault_radius_pct_extent,
	      fault_max_ang,
	      fault_boost_decay,
	      erosion_rate,
	      deposition_rate,
	      raindrop_friction,
	      raindrop_speed,
	      final_noise_amp,
	      final_noise_freq,
	      final_noise_lacu;
};

struct terrain {
	struct worldgen_opts opts;
	float height, width;
	float max_watershed;
	uint8_t faults;
	struct hdarr *tdat;
	struct darr *fault_points;
	struct pointf mid;
	float radius;
	struct terrain_pixel *heightmap;
};

struct gen_terrain_ctx {
	struct trigraph tg;
	struct terrain terra;
	struct {
		_Atomic bool points;
		_Atomic bool tris;
	} finished;
};

void gen_terrain_init(struct gen_terrain_ctx *ctx, const struct worldgen_opts *opts);
void gen_terrain_reset(struct gen_terrain_ctx *ctx, const struct worldgen_opts *opts);
void full_gen_terrain(struct chunks *chunks, struct gen_terrain_ctx *ctx);

void seed_points(struct gen_terrain_ctx *ctx);
void delauny_triangulation(struct gen_terrain_ctx *ctx);
void init_terrain_data(struct trigraph *tg, struct terrain *terra);
void gen_faults(struct gen_terrain_ctx *ctx);
void fill_plates(struct gen_terrain_ctx *ctx);
void rasterize_terrain(struct gen_terrain_ctx *ctx);
void simulate_erosion(struct terrain *terra);
void trace_rivers(struct terrain *terra);
void gaussian_blur(struct terrain *terra, float sigma, uint8_t r, uint8_t off, uint8_t depth);
void add_noise(struct terrain *terra);
void write_chunks(struct chunks *chunks, struct terrain *terra);
#endif
