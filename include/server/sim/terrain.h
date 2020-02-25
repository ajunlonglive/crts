#ifndef __TERRAIN_H
#define __TERRAIN_H

#include <stdbool.h>

#include "shared/sim/chunk.h"
#include "shared/types/geom.h"

struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
bool find_tile(enum tile t, struct chunks *cnks, struct circle *range, struct point *result);
bool is_traversable(struct chunks *cnks, const struct point *p);
#endif
