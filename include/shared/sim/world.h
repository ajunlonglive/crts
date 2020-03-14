#ifndef __WORLD_H
#define __WORLD_H

#include <stddef.h>

#include "shared/sim/chunk.h"
#include "shared/types/hdarr.h"

#ifdef CRTS_SERVER
#include "shared/types/darr.h"
#endif

struct world {
	struct hdarr *ents;
	struct chunks *chunks;
#ifdef CRTS_SERVER
	struct darr *graveyard;
#endif
	uint32_t seq;
};

struct world *world_init(void);
struct ent *world_spawn(struct world *w);
void world_despawn(struct world *w, uint32_t id);
#endif
