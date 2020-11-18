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

void ag_init_components(struct chunks *cnks);
void ag_reset_component(const struct chunk *ck, struct ag_component *agc,
	enum trav_type tt);
void ag_link_component(struct abstract_graph *ag, struct ag_component *agc);
void ag_print_component(struct ag_component *agc);
#endif
