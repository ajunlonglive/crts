#include "posix.h"

#include <stdlib.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/types/bheap.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"

struct ag_heap_e {
	uint32_t d;
	struct ag_key key;
};

// TODO: this can be fixed by changing the type of hash to uint64_t
_Static_assert(sizeof(union ag_val) == sizeof(size_t), "");

static void
trace_path(struct abstract_graph *ag, const union ag_val *goal, struct ag_path *path,
	uint8_t *pathlen, struct ag_component *agc_s,
	struct ag_component *agc_g, uint8_t sidx, uint8_t gidx)
{
	const union ag_val *cur;
	const struct ag_key *curk;
	struct ag_component *agc;

	path->comp[0] = agc_g->pos;
	path->node[0] = gidx;
	(*pathlen) = 1;

	cur = goal;

	do {
		curk = &cur->s.prev;

		if (curk->node == tmp_node) {
			break;
		}

		cur = (union ag_val *)hash_get(ag->visited, curk);
		agc = hdarr_get_by_i(ag->components, curk->component);

		path->comp[*pathlen] = agc->pos;
		path->node[*pathlen] = ag_component_node_indices[curk->node][0];

		(*pathlen)++;
	} while (1);

	path->comp[*pathlen] = agc_s->pos;
	path->node[*pathlen] = sidx;
}

static const uint64_t *
get_adj_ag_component(struct abstract_graph *ag, struct ag_component *agc, uint8_t node)
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
		return NULL;
	}

	return hdarr_get_i(ag->components, &cp);
}


#define POINT_TO_IDX(p) ((p.x << 4) + p.y)
#define IDX_TO_POS(i) ag_component_node_indices[i][0] / 16, ag_component_node_indices[i][0] % 16

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
					cp->y + (ag_component_node_indices[nbrk->node][0] & 15)
				};

				dx = goal->x - p.x;
				dy = goal->y - p.y;

				h = dx * dx + dy * dy;
			}

			/* L("-> neighbour node:%d:(%d, %d), comp:%d, %d", nbrk->node, */
			/* 	IDX_TO_POS(nbrk->node), nbrk->component, tmpd + h); */

			/* should heuristic be tmpd + h or just h ? */
			bheap_push(ag->heap, &(struct ag_heap_e){ .d =  h, .key = *nbrk });
		}
	}
}

bool
astar_abstract(struct abstract_graph *ag, const struct point *s,
	const struct point *g, struct ag_path *path, uint8_t *pathlen)
{
	bool found = false;
	const uint64_t *component_idx_ptr;
	uint16_t component_idx, ni;

	struct point cp_s = nearest_chunk(s),
		     cp_g = nearest_chunk(g),
		     rels = point_sub(s, &cp_s),
		     relg = point_sub(g, &cp_g);

	uint8_t start_idx = POINT_TO_IDX(rels), goal_idx = POINT_TO_IDX(relg);

	/* L("pathfinding: (%d, %d)(%d, %d)(%d, %d) -> (%d, %d)(%d, %d)(%d, %d)", */
	/* 	s->x, s->y, cp_s.x, cp_s.y, rels.x, rels.y, */
	/* 	g->x, g->y, cp_g.x, cp_g.y, relg.x, relg.y */
	/* 	); */

#ifndef NDEBUG
	if (!(hdarr_get(ag->components, &cp_s) && hdarr_get(ag->components, &cp_g))) {
		LOG_W("attempting to pathfind to/from unknown area");
		abort();
	}
#endif

	component_idx = *hdarr_get_i(ag->components, &cp_s);

	struct ag_component *agc_s = hdarr_get_by_i(ag->components, component_idx),
			    *agc_g = hdarr_get(ag->components, &cp_g),
			    oagc_s = *agc_s, oagc_g = *agc_g,
			    *cur_agc, *nbr_agc;

	if (points_equal(&cp_s, &cp_g)) {
		L("start and goal are in the same component");
		path->comp[0] = agc_g->pos;
		path->node[0] = goal_idx;
		path->comp[1] = agc_s->pos;
		path->node[1] = start_idx;
		*pathlen = 2;
		return true;
	}


	if (!insert_tmp_node(agc_s, start_idx)) {
		L("unable to connect start to edge");
		return false;
	}

	if (!insert_tmp_node(agc_g, goal_idx)) {
		L("unable to connect goal to edge");
		return false;
	}

	struct ag_key curk = { .component = component_idx, .node = tmp_node };
	struct ag_node *curn;

	struct ag_heap_e start = { .key = curk };

	/* resetting visited and heap here, rather than at the end of the
	 * function so that we can draw the debug pathfinding overlay */
	hash_clear(ag->visited);
	darr_clear(ag->heap);

	darr_push(ag->heap, &start);
	hash_set(ag->visited, &curk, 0);

	union ag_val cur;

	while (darr_len(ag->heap)) {
		curk = ((struct ag_heap_e *)darr_get(ag->heap, 0))->key;
		bheap_pop(ag->heap);

		cur = *(union ag_val *)hash_get(ag->visited, &curk);
		cur_agc = hdarr_get_by_i(ag->components, curk.component);
		curn = &cur_agc->nodes[curk.node];

		/* L("checking comp:%d @ (%d, %d), node:%d, (%d, %d)", curk.component, */
		/* 	cur_agc->pos.x, cur_agc->pos.y, curk.node, IDX_TO_POS(curk.node)); */

		if (cur_agc == agc_g && curk.node == tmp_node) {
			found = true;
			break;
		}

		for (ni = 0; ni < curn->edges; ++ni) {
			struct ag_key nbrk = {
				.component = curk.component,
				.node = curn->adjacent[ni]
			};

			check_neighbour(ag, curn->weights[ni], &nbrk, &curk, &cur, &cur_agc->pos, g);
		}

		if ((component_idx_ptr = get_adj_ag_component(ag, cur_agc, curk.node))) {
			assert(*component_idx_ptr < UINT16_MAX);

			nbr_agc = hdarr_get_by_i(ag->components, *component_idx_ptr);

			struct ag_key nbrk = {
				.component = *component_idx_ptr,
				.node = ag_component_node_indices[curk.node][3]
			};

			check_neighbour(ag, 1, &nbrk, &curk, &cur, &nbr_agc->pos, g);
		}
	}

	if (found) {
		trace_path(ag, &cur, path, pathlen, agc_s, agc_g, start_idx, goal_idx);
	}

	L("checked %ld nodes, found: %c", hash_len(ag->visited), found ? 'y' : 'n');

	hdarr_set(ag->components, s, &oagc_s);
	hdarr_set(ag->components, g, &oagc_g);

	return found;
}

void
abstract_graph_init(struct abstract_graph *ag)
{
	L("initializing ag, %ld", sizeof(struct ag_key));
	ag->components = hdarr_init(2048, sizeof(struct point),
		sizeof(struct ag_component), NULL);
	ag->visited = hash_init(2048, 1, sizeof(struct ag_key));
	ag->heap = darr_init(sizeof(struct ag_heap_e));

	ag->trav = trav_land;
}

void
abstract_graph_destroy(struct abstract_graph *ag)
{
	hdarr_destroy(ag->components);
	hash_destroy(ag->visited);
	darr_destroy(ag->heap);
}
