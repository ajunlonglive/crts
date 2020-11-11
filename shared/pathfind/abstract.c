#include "posix.h"

#include <stdlib.h>

#ifndef CRTS_PATHFINDING
#define CRTS_PATHFINDING
#endif

#include "shared/pathfind/abstract.h"
#include "shared/pathfind/api.h"
#include "shared/pathfind/local.h"
#include "shared/pathfind/macros.h"
#include "shared/pathfind/preprocess.h"
#include "shared/sim/chunk.h"
#include "shared/types/bheap.h"
#include "shared/types/hash.h"
#include "shared/util/log.h"
#include "tracy.h"

_Static_assert(sizeof(union ag_val) == sizeof(uint64_t), "");

static void
add_node_to_path(struct ag_path *path, struct point *agc_pos, uint8_t node, uint16_t *pl)
{
	if (*pl >= MAXPATH_ABSTRACT) {
		LOG_W("abstract path too long");
		return;
	}

	/* L("(%d, %d) @ (%d, %d)", agc_pos->x, agc_pos->y, node >> 4, node & 15); */

	path->comp[*pl] = *agc_pos;
	path->node[*pl] = node;
	++(*pl);
}

static uint64_t
get_adj_ag_component(struct abstract_graph *ag, struct ag_component *agc, enum adj_chunk side)
{
	struct point cp = agc->pos;

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

	return *hdarr_get_i(ag->components, &cp);
}

#define POINT_TO_IDX(p) ((p.x << 4) + p.y)

static void
check_neighbour(struct abstract_graph *ag,
	struct ag_key *nbrk, struct ag_key *curk,
	uint32_t d, const struct point *ctr,
	const struct point *goal)
{
	bool new;
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

	assert(nbr);

	if (d < nbr->s.d) {
		nbr->s.d = d;
		nbr->s.prev = *curk;

		if (new) {
			int32_t h = 0;

			int32_t dx, dy;

			dx = goal->x - ctr->x;
			dy = goal->y - ctr->y;

			h = dx * dx + dy * dy;

			/* L("(%d, %d), (%d, %d) %d, %d, => %d", p.x, p.y, goal->x, goal->y, dx, dy, h); */

			bheap_push(ag->heap, &(struct ag_heap_e){ .d = (d * d) + h, .key = *nbrk });
		}
	}
}

bool
astar_abstract(struct abstract_graph *ag, const struct point *s,
	const struct point *g, struct ag_path *path, uint16_t *pathlen)
{
	assert((path && pathlen) || (!path && !pathlen));

	TracyCZoneAutoS;
	uint16_t ni;

	struct point cp_s = nearest_chunk(s), cp_g = nearest_chunk(g), ctr;

	/* L("pathfinding: (%d, %d)(%d, %d)(%d, %d) -> (%d, %d)(%d, %d)(%d, %d)", */
	/* 	s->x, s->y, cp_s.x, cp_s.y, rels.x, rels.y, */
	/* 	g->x, g->y, cp_g.x, cp_g.y, relg.x, relg.y */
	/* 	); */

	uint16_t start_component = *hdarr_get_i(ag->components, &cp_s),
		 goal_component = *hdarr_get_i(ag->components, &cp_g);

	struct ag_component *cur_agc, *nbr_agc,
			    *agc_s = hdarr_get_by_i(ag->components, start_component),
			    *agc_g = hdarr_get_by_i(ag->components, goal_component);

	uint8_t start_idx = POINT_TO_IDX(point_sub(s, &cp_s)),
		goal_idx = POINT_TO_IDX(point_sub(g, &cp_g)),
		start_region = SB4_GET(agc_s->region_map, start_idx),
		goal_region = SB4_GET(agc_g->region_map, goal_idx);

	if (points_equal(&cp_s, &cp_g)
	    && astar_local_possible(agc_s, start_idx, goal_idx)) {
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

	struct ag_key curk = { .component = start_component,
			       .region = SB4_GET(agc_s->region_map, start_idx) },
		      nbrk;
	cur_agc = agc_s;
	struct ag_region *curn = &agc_s->regions[curk.region];

	/* resetting visited and heap here, rather than at the end of the
	 * function so that we can draw the debug pathfinding overlay */
	hash_clear(ag->visited);
	darr_clear(ag->heap);

	hash_set(ag->visited, &curk, (union ag_val){ .s = { .prev = { .region = 123 } } }.i);

	union ag_val cur = { 0 };

	while (1) {
		for (ni = 0; ni < curn->edges_len; ++ni) {
			nbrk = (struct ag_key){
				.component = get_adj_ag_component(ag, cur_agc, curn->edges[ni] & 3),
				.region = curn->edges[ni] >> 2,
				.edge_i = ni,
			};

			nbr_agc = hdarr_get_by_i(ag->components, nbrk.component);

			if (nbrk.component == goal_component
			    && nbrk.region == goal_region) {
				goto found;
			}

			ctr = (struct point) {
				cur_agc->pos.x + (nbr_agc->regions[nbrk.region].center >> 4),
				cur_agc->pos.y + (nbr_agc->regions[nbrk.region].center & 15)
			};

			check_neighbour(ag, &nbrk, &curk, cur.s.d, &ctr, g);
		}

		/* get next node */
		if (!darr_len(ag->heap)) {
			break;
		}

		curk = ((struct ag_heap_e *)darr_get(ag->heap, 0))->key;
		bheap_pop(ag->heap);

		cur = *(union ag_val *)hash_get(ag->visited, &curk);
		cur_agc = hdarr_get_by_i(ag->components, curk.component);

		curn = &cur_agc->regions[curk.region];
	}

	/* L("checked %ld nodes, found: no", hash_len(ag->visited)); */

	goto return_false;
found:
	/* L("checked %ld nodes, found: yes", hash_len(ag->visited)); */

	if (!path) {
		goto return_true;
	}

	*pathlen = 0;

	assert(points_equal(&agc_g->pos, &nbr_agc->pos));

	add_node_to_path(path, &agc_g->pos, goal_idx, pathlen);

	uint8_t entrance, exit;
	while (1) {
		entrance = cur_agc->regions[curk.region].entrances[nbrk.edge_i];

		switch (cur_agc->regions[curk.region].edges[nbrk.edge_i] & 3) {
		case adjck_left:
			exit = entrance + 240;
			break;
		case adjck_right:
			exit = entrance - 240;
			break;
		case adjck_up:
			exit = entrance + 15;
			break;
		case adjck_down:
			exit = entrance - 15;
			break;
		}

		add_node_to_path(path, &nbr_agc->pos, exit, pathlen);
		add_node_to_path(path, &cur_agc->pos, entrance, pathlen);

		if (curk.component == start_component && curk.region == start_region) {
			break;
		}

		nbrk = curk;
		curk = ((union ag_val *)hash_get(ag->visited, &curk))->s.prev;
		nbr_agc = cur_agc;
		cur_agc = hdarr_get_by_i(ag->components, curk.component);
	}

	add_node_to_path(path, &agc_s->pos, start_idx, pathlen);


return_true:
	TracyCZoneAutoE;
	return true;
return_false:
	TracyCZoneAutoE;
	return false;
}
