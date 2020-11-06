#include "posix.h"

#include <stdlib.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#define START_COMP (UINT16_MAX - 1)

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/types/bheap.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"
#include "tracy.h"

// TODO: this can be fixed by changing the type of hash to uint64_t
_Static_assert(sizeof(union ag_val) == sizeof(size_t), "");

static void
add_node_to_path(struct ag_path *path, struct point *agc_pos, uint8_t node, uint16_t *pl)
{
	if (*pl >= MAXPATH_ABSTRACT) {
		LOG_W("abstract path too long");
		return;
	}

	path->comp[*pl] = *agc_pos;
	path->node[*pl] = node;
	++(*pl);
}

static void
trace_path(struct abstract_graph *ag, const union ag_val *cur, const struct ag_key *curk,
	struct ag_path *path, uint16_t *pathlen,
	struct ag_component *agc_s, struct ag_component *agc_g,
	uint8_t sidx, uint8_t gidx)
{
	TracyCZoneAutoS;
	struct ag_component *agc;

	do {
		if (curk->component == START_COMP) {
			agc = agc_s;
		} else {
			agc = hdarr_get_by_i(ag->components, curk->component);
		}

		add_node_to_path(path, &agc->pos, ag_component_node_indices[curk->node][0], pathlen);

		curk = &cur->s.prev;

		if (curk->node == tmp_node) {
			break;
		}

		cur = (union ag_val *)hash_get(ag->visited, curk);
	} while (1);

	add_node_to_path(path, &agc_s->pos, sidx, pathlen);

	TracyCZoneAutoE;
}

static struct point
get_adj_ag_component(struct ag_component *agc, uint8_t node)
{
	struct point cp = agc->pos;
	enum adj_chunk side = ag_component_node_indices[node][1];

	switch (side) {
	case adjck_left:
		cp.x -= CHUNK_SIZE;
		break;
	case adjck_down:
		cp.y += CHUNK_SIZE;
		break;
	case adjck_right:
		cp.x += CHUNK_SIZE;
		break;
	case adjck_up:
		cp.y -= CHUNK_SIZE;
		break;
	case adjck_no:
		assert(false);
		break;
	}

	return cp;
}

#define POINT_TO_IDX(p) ((p.x << 4) + p.y)

static void
check_neighbour(struct abstract_graph *ag, uint32_t d, struct ag_key *nbrk,
	struct ag_key *curk, const union ag_val *cur, const struct point *cp,
	const struct point *goal)
{
	bool new;
	uint32_t tmpd;
	const uint64_t *v;
	union ag_val *nbr;


	if ((v = hash_get(ag->visited, nbrk))) {
		nbr = (union ag_val *)v;
		new = false;
	} else {
		hash_set(ag->visited, nbrk, (union ag_val){ .s = { .d = UINT16_MAX,
								   .prev = *curk } }.i);
		nbr = (union ag_val *)hash_get(ag->visited, nbrk);
		new = true;
	}

	assert(cur && nbr);

	if ((tmpd = cur->s.d + d) < nbr->s.d) {
		nbr->s.d = tmpd;
		nbr->s.prev = *curk;

		if (new) {
			int32_t h = 0;

			if (nbrk->node != tmp_node) {
				int32_t dx, dy;
				struct point p = {
					cp->x + (ag_component_node_indices[nbrk->node][0] >> 4),
					cp->y + (ag_component_node_indices[nbrk->node][0] & 15),
				};

				dx = goal->x - p.x;
				dy = goal->y - p.y;

				h = dx * dx + dy * dy;

				/* L("(%d, %d), (%d, %d) %d, %d, => %d", p.x, p.y, goal->x, goal->y, dx, dy, h); */
			}

			bheap_push(ag->heap, &(struct ag_heap_e){ .d = (tmpd * tmpd) + h, .key = *nbrk });
		}
	}
}

static bool
setup_tmp_component(struct abstract_graph *ag, const struct point *cp,
	uint8_t idx, uint64_t *ret)
{
	struct ag_component tmp;
	const uint64_t *agci;

	struct ag_tmp_component_key key = { .pos = *cp, .node = idx, };

	if (!(agci = hdarr_get_i(ag->tmp_components, &key))) {
		tmp = *(struct ag_component *)hdarr_get(ag->components, cp);

		if (!insert_tmp_node(&tmp, idx)) {
			L("unable to connect tmp component to edge");
			return false;
		}

		hdarr_set(ag->tmp_components, &key, &tmp);
		agci = hdarr_get_i(ag->tmp_components, &key);
		assert(agci);
	}

	*ret = *agci;

	return true;
}

bool
astar_abstract(struct abstract_graph *ag, const struct point *s,
	const struct point *g, struct ag_path *path, uint16_t *pathlen)
{
	assert((path && pathlen) || (!path && !pathlen));

	TracyCZoneAutoS;
	uint16_t ni;

	struct point cp_s = nearest_chunk(s), cp_g = nearest_chunk(g);

	uint8_t start_idx = POINT_TO_IDX(point_sub(s, &cp_s)),
		goal_idx = POINT_TO_IDX(point_sub(g, &cp_g));

	/* L("pathfinding: (%d, %d)(%d, %d)(%d, %d) -> (%d, %d)(%d, %d)(%d, %d)", */
	/* 	s->x, s->y, cp_s.x, cp_s.y, rels.x, rels.y, */
	/* 	g->x, g->y, cp_g.x, cp_g.y, relg.x, relg.y */
	/* 	); */

	/* wrong assumption */
	if (points_equal(&cp_s, &cp_g)) {
		if (!path) {
			goto return_true;
		}

		path->comp[0] = cp_g;
		path->node[0] = goal_idx;
		path->comp[1] = cp_s;
		path->node[1] = start_idx;
		*pathlen = 2;
		goto return_true;
	}


#ifndef NDEBUG
	if (!(hdarr_get(ag->components, &cp_s)
	      && hdarr_get(ag->components, &cp_g))) {
		LOG_W("attempting to pathfind to/from unknown area");
		abort();
	}
#endif

	uint64_t agc_si, agc_gi;

	if (!((setup_tmp_component(ag, &cp_s, start_idx, &agc_si))
	      && (setup_tmp_component(ag, &cp_g, goal_idx, &agc_gi)))) {
		goto return_false;
	}

	struct ag_component *cur_agc, *nbr_agc,
			    *agc_s = hdarr_get_by_i(ag->tmp_components, agc_si),
			    *agc_g = hdarr_get_by_i(ag->tmp_components, agc_gi);

	struct ag_key curk = { .component = START_COMP, .node = tmp_node };
	cur_agc = agc_s;
	struct ag_node *curn = &agc_s->nodes[tmp_node];

	/* resetting visited and heap here, rather than at the end of the
	 * function so that we can draw the debug pathfinding overlay */
	hash_clear(ag->visited);
	darr_clear(ag->heap);

	hash_set(ag->visited, &curk, 0);

	union ag_val cur = { 0 };

	goto handle_inner_neighbours;

	while (1) {
		/* handle outer neighbour */
		struct point adjp = get_adj_ag_component(cur_agc, curk.node);

		if (points_equal(&adjp, &cp_g)) {
			goto found;
		}

		struct ag_key nbrk = {
			.component = *hdarr_get_i(ag->components, &adjp),
			.node = ag_component_node_indices[curk.node][3]
		};

		nbr_agc = hdarr_get_by_i(ag->components, nbrk.component);

		// TODO: investigate why the below fails */
		/* assert(nbr_agc->nodes[nbrk.node].flags & agn_filled); */
		if ((nbr_agc->nodes[nbrk.node].flags & agn_filled)) {
			check_neighbour(ag, 1, &nbrk, &curk, &cur, &nbr_agc->pos, g);
		}

handle_inner_neighbours:
		/* handle inner neighbours */
		for (ni = 0; ni < curn->edges; ++ni) {
			struct ag_key nbrk = {
				.component = curk.component,
				.node = curn->adjacent[ni]
			};

			check_neighbour(ag, curn->weights[ni], &nbrk, &curk, &cur, &cur_agc->pos, g);
		}

		/* get next node */
		if (!darr_len(ag->heap)) {
			break;
		}

		curk = ((struct ag_heap_e *)darr_get(ag->heap, 0))->key;
		bheap_pop(ag->heap);

		cur = *(union ag_val *)hash_get(ag->visited, &curk);
		if (curk.component == START_COMP) {
			cur_agc = agc_s;
		} else {
			cur_agc = hdarr_get_by_i(ag->components, curk.component);
		}
		curn = &cur_agc->nodes[curk.node];
	}

	/* L("checked %ld nodes, found: no", hash_len(ag->visited)); */

	goto return_false;
found:
	/* L("checked %ld nodes, found: yes", hash_len(ag->visited)); */

	if (!path) {
		goto return_true;
	}

	*pathlen = 0;

	add_node_to_path(path, &agc_g->pos, goal_idx, pathlen);
	add_node_to_path(path, &agc_g->pos, ag_component_node_indices[curk.node][2], pathlen);

	trace_path(ag, &cur, &curk, path, pathlen, agc_s, agc_g, start_idx, goal_idx);

return_true:
	TracyCZoneAutoE;
	return true;
return_false:
	TracyCZoneAutoE;
	return false;
}
