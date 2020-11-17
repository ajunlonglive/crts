#ifndef SHARED_PATHFIND_API_H
#define SHARED_PATHFIND_API_H

#include "shared/sim/chunk.h"
#include "shared/types/result.h"

struct abstract_graph;

void hpa_finish(struct chunks *cnks, uint32_t path);
bool hpa_start(struct chunks *cnks, const struct point *s, const struct point *g,
	uint32_t *handle);
enum result hpa_continue(struct chunks *cnks, uint32_t path, struct point *p);
bool hpa_path_exists(struct chunks *cnks, const struct point *s, const struct point *g);
void hpa_dirty_point(struct chunks *cnks, const struct point *p);
void hpa_clean(struct chunks *cnks);
void abstract_graph_init(struct abstract_graph *ag);
void abstract_graph_destroy(struct abstract_graph *ag);
#endif
