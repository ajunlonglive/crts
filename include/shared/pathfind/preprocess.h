#ifndef SHARED_PATHFIND_PREPROCESS_H
#define SHARED_PATHFIND_PREPROCESS_H

#include "shared/pathfind/abstract.h"

enum adj_chunk {
	adjck_left,
	adjck_down,
	adjck_right,
	adjck_up,
	adjck_no,
};

extern const uint32_t ag_component_node_indices[CHUNK_PERIM + 1][4];

bool insert_tmp_node(struct ag_component *agc, uint8_t tmp_node_i);
void ag_preprocess_chunk(struct chunks *cnks, struct chunk *ck);
#endif
