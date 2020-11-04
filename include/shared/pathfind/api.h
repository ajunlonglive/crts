#ifndef SHARED_PATHFIND_API_H
#define SHARED_PATHFIND_API_H

#include "shared/sim/chunk.h"
#include "shared/types/result.h"

enum pathfind_path_flags {
	ppf_local_done    = 1 << 0,
	ppf_abstract_done = 1 << 1,
};

struct pathfind_path {
	struct ag_path abstract;
	uint8_t local[MAXPATH_LOCAL];
	struct point goal;
	uint8_t local_i, local_len, flags;
	uint16_t abstract_i, abstract_len;
};

bool hpa_start(struct chunks *cnks, struct pathfind_path *path, struct point *s, struct point *g);
enum result hpa_continue(struct chunks *cnks, struct pathfind_path *path, struct point *p);
#endif
