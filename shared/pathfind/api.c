#include "posix.h"

#include <assert.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#include "shared/pathfind/api.h"
#include "shared/pathfind/local.h"
#include "shared/sim/chunk.h"
#include "shared/util/log.h"

#define CUR_COMP path->abstract.comp[path->abstract_i]
#define NXT_COMP path->abstract.comp[path->abstract_i - 1]
#define CUR_ABS_NODE path->abstract.node[path->abstract_i]
#define NXT_ABS_NODE path->abstract.node[path->abstract_i - 1]
#define CUR_LOC_NODE path->local[path->local_i]
#define NXT_LOC_NODE path->local[path->local_i - 1]

#define IDX_TO_POS(i) (struct point){ i & 15, i >> 4 }

bool
hpa_start(struct chunks *cnks, struct pathfind_path *path, struct point *s, struct point *g)
{
	if (!astar_abstract(&cnks->ag, s, g, &path->abstract, &path->abstract_i)) {
		return false;
	}

	assert(path->abstract_i >= 2);

	path->abstract_i--;

	struct ag_component *agc = hdarr_get(cnks->ag.components, &CUR_COMP);
	assert(agc);

	if (!astar_local(agc, CUR_ABS_NODE, NXT_ABS_NODE, path->local,
		&path->local_i)) {
		/* assert(false); */
		return false;
	}

	path->abstract_i--;

	return true;
}

enum result
hpa_continue(struct chunks *cnks, struct pathfind_path *path, struct point *p)
{
	struct ag_component *agc;

	/* if (we need to recalculate path ) { */
	/* 	recalculate_path() */
	/* } */

	if (!path->local_i) {
		L("end of local graph");
		if (!path->abstract_i) {
			L("end of abstract graph, done!")
			return rs_done;
		} else if (points_equal(&CUR_COMP, &NXT_COMP)) {
			L("finding a new graph in %d %d", CUR_COMP.x, CUR_COMP.y);
			agc = hdarr_get(cnks->ag.components, &CUR_COMP);
			assert(agc);

			if (!astar_local(agc, CUR_ABS_NODE, NXT_ABS_NODE,
				path->local, &path->local_i)) {
				LOG_W("oops... failed");
				assert(false);
			}

			path->abstract_i--;
		} else {
			L("shifting component");
			*p = IDX_TO_POS(NXT_ABS_NODE);
			path->abstract_i--;
			return rs_cont;
		}
	}

	*p = IDX_TO_POS(NXT_LOC_NODE);

	path->local_i--;

	return rs_cont;
}
