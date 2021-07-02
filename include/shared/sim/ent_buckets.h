#ifndef SERVER_SIM_ENT_BUCKETS_H
#define SERVER_SIM_ENT_BUCKETS_H

#include "shared/sim/ent.h"

struct ent_buckets {
	struct darr buckets;
	struct hash keys;
};

typedef enum iteration_result ((*for_each_ent_at_cb)(void *ctx, struct ent *e));

/* void ent_buckets_unset(struct ent_buckets *eb, uint32_t e_id, struct point *p); */
/* void ent_buckets_set(struct ent_buckets *eb, uint32_t e_id, struct point *p); */
void make_ent_buckets(struct ent_buckets *eb, struct hdarr *ents);
void ent_buckets_init(struct ent_buckets *eb);
void for_each_ent_at(struct ent_buckets *eb, struct hdarr *ents, const struct point *p,
	void *ctx, for_each_ent_at_cb func);
#endif
