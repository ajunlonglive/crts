#ifndef __PATHFIND_HEAP_H
#define __PATHFIND_HEAP_H

#include "server/sim/pathfind/pgraph.h"

int heap_push(struct path_graph *pg, const struct node *n);
void heap_init(struct path_graph *pg);
int heap_pop(struct path_graph *pg);
void heap_sort(struct path_graph *pg);
struct node *heap_peek(const struct path_graph *pg);
#endif
