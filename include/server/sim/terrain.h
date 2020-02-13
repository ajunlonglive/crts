#ifndef __TERRAIN_H
#define __TERRAIN_H

#include "shared/sim/chunk.h"
#include "shared/types/geom.h"

void init_terrain_gen(void);
const struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
#endif
