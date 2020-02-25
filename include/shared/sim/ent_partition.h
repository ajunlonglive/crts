#ifndef TYPES_ENT_PARTITION_H
#define TYPES_ENT_PARTITION_H

#include "shared/sim/world.h"
#include "shared/types/hdarr.h"

struct ent_partition;

struct ent_partition *ent_partition_init(const struct world *w, uint16_t partition_size);
void ent_partition_destroy(struct ent_partition *ep);
#endif
