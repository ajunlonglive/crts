#ifndef GENWORLD_WORKER_H
#define GENWORLD_WORKER_H
#include "genworld/gen.h"

enum gen_terrain_step {
	gts_triangulate,
	gts_faults,
	gts_raster,
	gts_erosion,
	gts_blur,
};

void init_genworld_worker(struct gen_terrain_ctx *gt);
void start_genworld_worker(struct gen_terrain_ctx *gt, struct worldgen_opts *opts,
	enum gen_terrain_step step);
#endif
