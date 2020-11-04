#include "posix.h"

#include "client/hiface.h"
#include "client/input/debug.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/preprocess.h"
#include "shared/util/log.h"


void
debug_pathfind_toggle(struct hiface *hf)
{
	if (hf->keymap_describe) {
		hf_describe(hf, kmc_debug, "enable debug pathfinding mode");
		return;
	}

	uint32_t i;

	if ((hf->debug_path.on = !hf->debug_path.on)) {
		for (i = 0; i < hdarr_len(hf->sim->w->chunks.hd); ++i) {
			ag_preprocess_chunk(&hf->sim->w->chunks, hdarr_get_by_i(hf->sim->w->chunks.hd, i));
		}

		struct point c = point_add(&hf->view, &hf->cursor);
		hf->debug_path.goal = c;
		L("adding goal @ %d, %d", c.x, c.y);
	}

	hf->sim->changed.chunks = true;
}

void
debug_pathfind_place_point(struct hiface *hf)
{
	if (hf->keymap_describe) {
		hf_describe(hf, kmc_debug, "place the starting point for the path");
		return;
	}

	if (!hf->debug_path.on) {
		return;
	}

	struct point c = point_add(&hf->view, &hf->cursor);

	hpa_start(&hf->sim->w->chunks,
		&hf->debug_path.path, &c,
		&hf->debug_path.goal);

	darr_clear(hf->debug_path.path_points);

	darr_push(hf->debug_path.path_points, &c);

	uint32_t i, duplicates = 0;

	while ((hpa_continue(&hf->sim->w->chunks,
		&hf->debug_path.path, &c)) == rs_cont) {

		for (i = 0; i < darr_len(hf->debug_path.path_points); ++i) {
			struct point *d = darr_get(hf->debug_path.path_points, i);
			if (points_equal(&c, d)) {
				++duplicates;
			}
		}

		darr_push(hf->debug_path.path_points, &c);
	}

	L("duplicates in path: %d", duplicates);

	hf->sim->changed.chunks = true;
}
