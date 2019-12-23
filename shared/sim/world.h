#ifndef __WORLD_H
#define __WORLD_H
#include <stdlib.h>
#include "sim/chunk.h"

#define ENT_STEP 100

struct world {
	size_t ecnt;
	size_t ecap;
	struct ent *ents;
	struct chunks *chunks;
};

struct world *world_init(void);
struct ent *world_spawn(struct world *w);
void world_despawn(struct world *w, int i);
#endif
