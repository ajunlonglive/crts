#ifndef SHARED_PATHFIND_PREPROCESS_H
#define SHARED_PATHFIND_PREPROCESS_H

#include "shared/pathfind/abstract.h"
#include "shared/sim/chunk.h"

enum adj_chunk {
	adjck_left,
	adjck_down,
	adjck_right,
	adjck_up,
	adjck_no,
};

extern const uint32_t ag_component_node_indices[CHUNK_PERIM + 1][4];

void ag_init_components(struct chunks *cnks);
#endif
