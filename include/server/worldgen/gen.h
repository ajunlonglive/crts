#ifndef SERVER_SIM_GEN_TERRAIN_H
#define SERVER_SIM_GEN_TERRAIN_H

#include "shared/sim/chunk.h"

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


void gen_terrain(struct chunks *chunks, struct worldgen_opts *opts);
#endif
