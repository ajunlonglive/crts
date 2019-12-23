#ifndef __TERRAIN_H
#define __TERRAIN_H
#include "types/geom.h"
#include "sim/chunk.h"

void init_terrain_gen(void);
const struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
#endif
