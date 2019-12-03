#ifndef __TERRAIN_H
#define __TERRAIN_H
#include "types/geom.h"
#include "sim/world.h"

void init_terrain_gen(void);
struct chunk *get_chunk(struct world *w, struct point *p);
#endif
