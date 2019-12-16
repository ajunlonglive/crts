#ifndef __TERRAIN_H
#define __TERRAIN_H
#include "types/geom.h"
#include "sim/world.h"

void init_terrain_gen(void);
const struct chunk *get_chunk(struct hash *chunks, struct point *p);
#endif
