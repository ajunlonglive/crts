#ifndef SERVER_SIM_ENT_BUCKETS_H
#define SERVER_SIM_ENT_BUCKETS_H

#define BUCKET_SIZE 2

#include <stddef.h>

#include "shared/math/geom.h"
#include "shared/sim/ent.h"
#include "shared/types/hdarr.h"

struct ent_buckets {
	struct darr *buckets;
	struct hash *keys;
	struct hash *counts;
	size_t total;
};

/*
 * Layout:
 *
 * 0       1       2       3       4       5         index
 * | ent a | ent b | ent c | ent d | ent e | ent f   buckets, array of ent ids
 * | 0, 1  | 2, 2  | 3, 2  | 3, 2  | 4, 1  | 5, 1
 * |       |	           |
 *  \____   \_______        \___
 *       ^          ^           ^
 *  0, 0 = 1 | 2, 2 = 1 |  4, 0 = 3                  keys, hash point->index
 *
 */

void make_ent_buckets(struct hdarr *ents, struct ent_buckets *eb);
void ent_buckets_init(struct ent_buckets *eb);
void ent_buckets_clear(struct ent_buckets *eb);
void for_each_ent_at(struct ent_buckets *eb, struct hdarr *ents, const struct point *p,
	void *ctx, enum iteration_result ((*func)(void *ctx, struct ent *e)));
#endif
