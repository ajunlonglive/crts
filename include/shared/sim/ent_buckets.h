#ifndef SERVER_SIM_ENT_BUCKETS_H
#define SERVER_SIM_ENT_BUCKETS_H

#include "shared/sim/ent.h"

typedef enum iteration_result ((*for_each_ent_at_cb)(void *ctx, struct ent *e));

/* void ent_buckets_unset(struct ent_buckets *eb, uint32_t e_id, struct point *p); */
/* void ent_buckets_set(struct ent_buckets *eb, uint32_t e_id, struct point *p); */
void make_ent_buckets(struct hash *eb, struct hdarr *ents);
void ent_buckets_init(struct hash *eb);
void for_each_ent_adjacent_to(struct hash *eb, struct hdarr *ents, const struct ent *e, void *ctx, for_each_ent_at_cb func);
#endif
