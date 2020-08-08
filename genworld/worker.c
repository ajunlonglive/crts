#include "posix.h"

#include <pthread.h>
#include <string.h>

#include "genworld/worker.h"
#include "shared/types/darr.h"
#include "shared/util/log.h"

pthread_t worker_thread = { 0 };
pthread_attr_t worker_thread_attr = { 0 };

struct worker_ctx {
	struct gen_terrain_ctx *gt;
	enum gen_terrain_step step;
};

static void *
genworld_worker(void *_ctx)
{
	struct worker_ctx *ctx = _ctx;
	struct gen_terrain_ctx *gt = ctx->gt;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	switch (ctx->step) {
	case gts_triangulate:
		seed_points(gt);
		delauny_triangulation(gt);
	/* FALLTHROUGH */
	case gts_faults:
		L("gen faults");
		gen_faults(gt);
		fill_plates(gt);
	/* FALLTHROUGH */
	case gts_raster:
	/* FALLTHROUGH */
	case gts_erosion:
	/* FALLTHROUGH */
	case gts_blur:
		break;
	}

	return NULL;
}

void
init_genworld_worker(struct gen_terrain_ctx *gt)
{
	pthread_attr_init(&worker_thread_attr);
}

bool init = true;

void
start_genworld_worker(struct gen_terrain_ctx *gt, struct worldgen_opts *opts,
	enum gen_terrain_step step)
{
	if (init) {
		gen_terrain_init(gt, opts);
		init = false;
	} else {
		pthread_cancel(worker_thread);

		switch (step) {
		case gts_faults:
			gt->terra.opts = *opts;
			L("clearing faults");
			hdarr_clear(gt->terra.tdat);
			darr_clear(gt->terra.fault_points);
			init_terrain_data(&gt->tg, &gt->terra);
			/* size_t heightmap_size = opts->height * opts->width */
			/* 			* sizeof(struct terrain_pixel); */
			/* memset(gt->terra.heightmap, 0, heightmap_size); */
			break;
		default:
			gen_terrain_reset(gt, opts);
		}
	}

	static struct worker_ctx ctx;
	ctx = (struct worker_ctx){ .gt = gt, .step = step };

	pthread_create(&worker_thread, &worker_thread_attr, genworld_worker, &ctx);
}
