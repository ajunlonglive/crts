#ifndef __PATHFIND_PGRAPH_H
#define __PATHFIND_PGRAPH_H

#include <stddef.h>

#include "shared/types/geom.h"
#include "../../../../lib/gheap/gheap.h"

struct pgraph {
	struct {
		struct gheap_ctx ctx;
		uint16_t *e;
		size_t len;
		size_t cap;
	} heap;

	struct {
		struct pg_node *e;
		size_t len;
		size_t cap;
	} nodes;

	struct point goal;

	struct chunks *chunks;
	struct hash *hash;

	uint8_t possible;
};

struct pg_node *pgraph_lookup(const struct pgraph *g, const struct point *p);
struct pgraph * pgraph_create(struct chunks *cnks, const struct point *goal);
void pgraph_destroy(struct pgraph *pg);
#endif
