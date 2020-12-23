#ifndef __WORLD_H
#define __WORLD_H

#include <stddef.h>

#include "shared/sim/chunk.h"
#include "shared/types/darr.h"
#include "shared/types/hdarr.h"

struct world {
	struct chunks chunks;
	struct hdarr ents;
	struct darr spawn;
	struct darr graveyard;
	uint32_t seq;
};

void world_init(struct world *w);
void world_despawn(struct world *w, uint32_t id);
#endif
