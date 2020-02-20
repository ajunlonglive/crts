#ifndef __WORLD_H
#define __WORLD_H

#include <stddef.h>

#include "shared/sim/chunk.h"

struct world {
	struct {
		size_t len;
		size_t cap;
		struct ent *e;
	} ents;
	struct chunks *chunks;
};

struct world *world_init(void);
struct ent *world_spawn(struct world *w);
void world_despawn(struct world *w, size_t index);
#endif
