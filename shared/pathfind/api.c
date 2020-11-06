#include "posix.h"

#include <assert.h>
#include <string.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/abstract_graph.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/local.h"
#include "shared/pathfind/path.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

void
abstract_graph_init(struct abstract_graph *ag)
{
	L("initializing ag, %ld", sizeof(struct ag_key));
	ag->components = hdarr_init(2048, sizeof(struct point),
		sizeof(struct ag_component), NULL);
	ag->visited = hash_init(2048, 1, sizeof(struct ag_key));
	ag->heap = darr_init(sizeof(struct ag_heap_e));

	ag->tmp_components = hdarr_init(2048, sizeof(struct ag_tmp_component_key),
		sizeof(struct ag_component), NULL);
	ag->paths = darr_init(sizeof(struct pathfind_path));

	ag->free_paths = darr_init(sizeof(uint32_t));

	ag->trav = trav_land;
}

void
abstract_graph_destroy(struct abstract_graph *ag)
{
	hdarr_destroy(ag->components);
	hash_destroy(ag->visited);
	darr_destroy(ag->heap);
	hdarr_destroy(ag->tmp_components);
	darr_destroy(ag->paths);
}

void
hpa_finish(struct chunks *cnks, uint32_t path_id)
{
	struct pathfind_path *path = darr_get(cnks->ag.paths, path_id);

	if (path->flags & ppf_initialized) {
		/* L("pruning path %d", path_id); */

		memset(path, 0, sizeof(struct pathfind_path));

		darr_push(cnks->ag.free_paths, &path_id);
	}
}

bool
hpa_start(struct chunks *cnks, const struct point *s, const struct point *g,
	uint32_t *handle)
{
	size_t free_len;
	uint32_t id;
	struct pathfind_path *path;

	if ((free_len = darr_len(cnks->ag.free_paths))) {
		id = *(uint32_t *)darr_get(cnks->ag.free_paths, free_len - 1);
		darr_del(cnks->ag.free_paths, free_len - 1);

		path = darr_get(cnks->ag.paths, id);
	} else {
		assert(darr_len(cnks->ag.paths) < UINT32_MAX);

		id = darr_len(cnks->ag.paths);

		/* L("creating new path slot @ %d", id); */
		path = darr_get_mem(cnks->ag.paths);
	}

	/* L("got path handle: %d", id); */

	assert(!(path->flags & ppf_initialized));
	path->flags |= ppf_initialized;

	if (!astar_abstract(&cnks->ag, s, g, &path->abstract, &path->abstract_len)) {
		hpa_finish(cnks, id);
		*handle = -1;
		return false;
	}

	assert(path->abstract_len >= 2);

	path->abstract_i = path->abstract_len - 1;

	path->flags |= ppf_local_done;

	*handle = id;

	return true;
}

enum result
hpa_continue(struct chunks *cnks, uint32_t id, struct point *p)
{
	struct pathfind_path *path = darr_get(cnks->ag.paths, id);

	assert(path->flags & ppf_initialized);

	struct ag_component *agc;

	/* if (we need to recalculate path ) { */
	/* 	recalculate_path() */
	/* } */

	if (path->flags & ppf_local_done) {
		struct point *cur_cmp = &path->abstract.comp[path->abstract_i],
			     *nxt_cmp = &path->abstract.comp[path->abstract_i - 1];

		assert(points_equal(cur_cmp, nxt_cmp));

		uint8_t cur_node = path->abstract.node[path->abstract_i],
			nxt_node = path->abstract.node[path->abstract_i - 1];

		/* L("(%d, %d) %d, %d", cur_cmp->x, cur_cmp->y, cur_node, nxt_node); */

		agc = hdarr_get(cnks->ag.components, cur_cmp);
		assert(agc);

		if (!astar_local(agc, cur_node, nxt_node,
			path->local, &path->local_len)) {
			LOG_W("failed to pathfind locally, the abstract path is invalid");
			assert(false);
		}

		if (path->abstract_i == 1) {
			path->abstract_i = 0;
			path->flags |= ppf_abstract_done;
		} else {
			path->abstract_i -= 2;
		}

		assert(path->local_len);
		path->flags &= ~ppf_local_done;
		path->local_i = path->local_len - 1;
	}

	p->x = path->abstract.comp[path->abstract_i + 1].x + (path->local[path->local_i] >> 4);
	p->y = path->abstract.comp[path->abstract_i + 1].y + (path->local[path->local_i] & 15);

	if (path->local_i) {
		path->local_i--;
	} else {
		if (path->flags & ppf_abstract_done) {
			/* L("done pathfining, calling hpa_finish"); */
			hpa_finish(cnks, id);

			return rs_done;
		}

		path->flags |= ppf_local_done;
	}

	return rs_cont;
}

bool
hpa_path_exists(struct chunks *cnks, const struct point *s, const struct point *g)
{
	return astar_abstract(&cnks->ag, s, g, NULL, NULL);
}
