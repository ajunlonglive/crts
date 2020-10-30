#ifndef SHARED_PATHFIND_API_H
#define SHARED_PATHFIND_API_H

#include "shared/sim/chunk.h"
#include "shared/types/result.h"

struct pathfind_path {
	struct ag_path abstract;
	uint8_t local[MAXPATH];
	struct point goal;
	uint8_t abstract_i, local_i,
		abstract_len, local_len;
};

bool hpa_start(struct chunks *cnks, struct pathfind_path *path, struct point *s, struct point *g);
enum result hpa_continue(struct chunks *cnks, struct pathfind_path *path, struct point *p);
#endif
