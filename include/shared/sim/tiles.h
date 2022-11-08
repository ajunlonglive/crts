#ifndef SHARED_SIM_TILES_H
#define SHARED_SIM_TILES_H

#include "shared/sim/chunk.h"
extern const struct cfg_lookup_table ltbl_tiles;

bool is_traversable(struct chunks *cnks, const struct point *p, uint8_t trav);
bool tile_is_traversable(enum tile t, uint8_t trav);
enum tile get_tile_at(struct chunks *cnks, const struct point *p);
float get_height_at(struct chunks *cnks, const struct point *p);
#endif
