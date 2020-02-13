#ifndef __PATHFIND_MAPPING_H
#define __PATHFIND_MAPPING_H
#include "pgraph.h"

void get_adjacent(struct path_graph *pg, struct node *n);
int brushfire(struct path_graph *pg, struct path_graph *guide, const struct point *e);
void reset_graph_hdist(struct path_graph *pg, const struct point *p);
#endif
