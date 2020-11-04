#include "posix.h"

#include <assert.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#include "shared/pathfind/api.h"
#include "shared/pathfind/local.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

bool
hpa_start(struct chunks *cnks, struct pathfind_path *path, struct point *s, struct point *g)
{
	if (!astar_abstract(&cnks->ag, s, g, &path->abstract, &path->abstract_len)) {
		return false;
	}


	assert(path->abstract_len >= 2);

	path->abstract_i = path->abstract_len - 1;

	path->flags = ppf_local_done;

	return true;
}

enum result
hpa_continue(struct chunks *cnks, struct pathfind_path *path, struct point *p)
{
	struct ag_component *agc;

	/* if (we need to recalculate path ) { */
	/* 	recalculate_path() */
	/* } */

	if (path->flags & ppf_local_done) {
		if (path->flags & ppf_abstract_done) {
			return rs_done;
		}

		struct point *cur_cmp = &path->abstract.comp[path->abstract_i],
			     *nxt_cmp = &path->abstract.comp[path->abstract_i - 1];

		assert(points_equal(cur_cmp, nxt_cmp));

		uint8_t cur_node = path->abstract.node[path->abstract_i],
			nxt_node = path->abstract.node[path->abstract_i - 1];

		agc = hdarr_get(cnks->ag.components, cur_cmp);
		assert(agc);

/* 		L("pathfinding locally, %d:(%d, %d) -> %d:(%d, %d)", */
/* 			cur_node, */
/* 			cur_node >> 4, */
/* 			cur_node & 15, */
/* 			nxt_node, */
/* 			nxt_node >> 4, */
/* 			nxt_node & 15); */

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
		path->flags |= ppf_local_done;
	}


	return rs_cont;
}
