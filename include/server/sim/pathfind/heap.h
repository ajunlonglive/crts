#ifndef __PATHFIND_HEAP_H
#define __PATHFIND_HEAP_H

#include "server/sim/pathfind/pg_node.h"
#include "server/sim/pathfind/pgraph.h"

size_t heap_push(struct pgraph *pg, const struct pg_node *n);
size_t heap_pop(struct pgraph *pg);
void heap_sort(struct pgraph *pg);
struct pg_node *heap_peek(const struct pgraph *pg);
#endif
