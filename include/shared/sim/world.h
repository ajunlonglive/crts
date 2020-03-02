#ifndef __WORLD_H
#define __WORLD_H

#include <stddef.h>

#include "shared/sim/chunk.h"
#include "shared/types/hdarr.h"

struct world {
	struct hdarr *ents;
	struct chunks *chunks;
	uint32_t seq;
};

struct world *world_init(void);
struct ent *world_spawn(struct world *w);
void world_despawn(struct world *w, uint32_t id);
#endif
