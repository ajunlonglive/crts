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
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"
#include "tracy.h"

void
abstract_graph_init(struct abstract_graph *ag)
{
	L("initializing ag, %ld", sizeof(struct ag_key));

	ag->components = hdarr_init(2048, sizeof(struct point),
		sizeof(struct ag_component), NULL);

	ag->visited = hash_init(2048, 1, sizeof(struct ag_key));

	ag->heap = darr_init(sizeof(struct ag_heap_e));

	ag->paths = darr_init(sizeof(struct pathfind_path));

	ag->free_paths = darr_init(sizeof(uint32_t));

	ag->dirty = hdarr_init(2048, sizeof(struct point), sizeof(struct ag_component_dirt), NULL);

	ag->trav = trav_land;
}

void
abstract_graph_destroy(struct abstract_graph *ag)
{
	hdarr_destroy(ag->components);
	hash_destroy(ag->visited);
	darr_destroy(ag->heap);
	darr_destroy(ag->paths);
	hdarr_destroy(ag->dirty);
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

static bool
setup_path(struct chunks *cnks, uint32_t id, struct pathfind_path *path, const struct point *s)
{
	if (!astar_abstract(&cnks->ag, s, &path->goal, &path->abstract, &path->abstract_len)) {
		hpa_finish(cnks, id);
		return false;
	}

	assert(path->abstract_len >= 2);

	path->abstract_i = path->abstract_len - 1;

	path->flags |= ppf_local_done;

	return true;
}

bool
hpa_start(struct chunks *cnks, const struct point *s, const struct point *g,
	uint32_t *handle)
{
	TracyCZoneAutoS;
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

	path->goal = *g;

	TracyCZoneAutoE;
	if (!setup_path(cnks, id, path, s)) {
		*handle = -1;
		return false;
	}

	*handle = id;

	TracyCZoneAutoE;
	return true;
}

enum result
hpa_continue(struct chunks *cnks, uint32_t id, struct point *p)
{
	TracyCZoneAutoS;
	struct pathfind_path *path = darr_get(cnks->ag.paths, id);

	assert(path->flags & ppf_initialized);

	struct ag_component *agc;

	if (path->flags & ppf_dirty) {
		struct point g = path->goal;
		*path = (struct pathfind_path){ .goal = g };

		if (!setup_path(cnks, id, path, p)) {
			TracyCZoneAutoE;
			return rs_fail;
		}

		path->flags |= ppf_initialized;
	}

	if (path->flags & ppf_local_done) {
		struct point *cur_cmp = &path->abstract.comp[path->abstract_i],
			     *nxt_cmp = &path->abstract.comp[path->abstract_i - 1];

		assert(points_equal(cur_cmp, nxt_cmp));

		uint8_t cur_node = path->abstract.node[path->abstract_i],
			nxt_node = path->abstract.node[path->abstract_i - 1];

		/* L("(%d, %d) | (%d, %d) -> (%d, %d)", cur_cmp->x, cur_cmp->y, */
		/* 	cur_node >> 4, */
		/* 	cur_node & 15, */
		/* 	nxt_node >> 4, */
		/* 	nxt_node & 15 */
		/* 	); */

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

			TracyCZoneAutoE;
			return rs_done;
		}

		path->flags |= ppf_local_done;
	}

	TracyCZoneAutoE;
	return rs_cont;
}

bool
hpa_path_exists(struct chunks *cnks, const struct point *s, const struct point *g)
{
	TracyCZoneAutoS;
	bool ret = astar_abstract(&cnks->ag, s, g, NULL, NULL);
	TracyCZoneAutoE;
	return ret;
}

void
hpa_dirty_point(struct chunks *cnks, const struct point *p)
{
	struct ag_component_dirt dirt = {
		.p = nearest_chunk(p), .type = dt_full
	};

	hdarr_set(cnks->ag.dirty, &dirt.p, &dirt);

	dirt.type = dt_relink;
	dirt.p.x -= CHUNK_SIZE;
	if (hdarr_get(cnks->ag.components, &dirt.p)) {
		hdarr_set(cnks->ag.dirty, &dirt.p, &dirt);
	}

	dirt.p.x += 2 * CHUNK_SIZE;
	if (hdarr_get(cnks->ag.components, &dirt.p)) {
		hdarr_set(cnks->ag.dirty, &dirt.p, &dirt);
	}

	dirt.p.x -= CHUNK_SIZE;
	dirt.p.y -= CHUNK_SIZE;
	if (hdarr_get(cnks->ag.components, &dirt.p)) {
		hdarr_set(cnks->ag.dirty, &dirt.p, &dirt);
	}

	dirt.p.y += 2 * CHUNK_SIZE;
	if (hdarr_get(cnks->ag.components, &dirt.p)) {
		hdarr_set(cnks->ag.dirty, &dirt.p, &dirt);
	}
}

void
hpa_clean(struct chunks *cnks)
{
	if (!hdarr_len(cnks->ag.dirty)) {
		return;
	}

	uint32_t i, j;
	struct pathfind_path *path;
	struct ag_component_dirt *dirt;
	struct ag_component *agc;
	struct chunk *ck;

	for (i = 0; i < hdarr_len(cnks->ag.dirty); ++i) {
		dirt = hdarr_get_by_i(cnks->ag.dirty, i);

		if (dirt->type != dt_full) {
			continue;
		}

		agc = hdarr_get(cnks->ag.components, &dirt->p);
		ck = hdarr_get(cnks->hd, &dirt->p);

		/* L("resetting component, (%d, %d)", agc->pos.x, agc->pos.y); */
		ag_reset_component(ck, agc, cnks->ag.trav);
	}

	for (i = 0; i < hdarr_len(cnks->ag.dirty); ++i) {
		dirt = hdarr_get_by_i(cnks->ag.dirty, i);

		agc = hdarr_get(cnks->ag.components, &dirt->p);

		if (dirt->type != dt_full) {
			for (j = 0; j < agc->regions_len; ++j) {
				agc->regions[j].edges_len = 0;
			}
		}

		/* L("relinking component, (%d, %d)", agc->pos.x, agc->pos.y); */
		ag_link_component(&cnks->ag, agc);
	}

	for (i = 0; i < darr_len(cnks->ag.paths); ++i) {
		path = darr_get(cnks->ag.paths, i);

		if (!(path->flags & ppf_initialized)) {
			continue;
		}

		/* start at abstract_i + 1 so we don't check already traversed
		* nodes.  abstract_i + 1 is guarenteed to be < abstract_len */
		assert(path->abstract_i + 1 < path->abstract_len);
		assert(path->abstract_i + 1 > 0);

		for (j = 0; j < (uint16_t)(path->abstract_i + 1); j += 2) {
			// TODO do something clever here
			/* for (j = 0; j < path->abstract_len; ++j) { */
			/* L("checking path el %d, component (%d, %d)", j, path->abstract.comp[j].x, path->abstract.comp[j].y); */
			if (hdarr_get(cnks->ag.dirty, &path->abstract.comp[j])) {
				/* L("resetting path %d", i); */
				path->flags |= ppf_dirty;
				break;
			}
		}
	}

	hdarr_clear(cnks->ag.dirty);
}