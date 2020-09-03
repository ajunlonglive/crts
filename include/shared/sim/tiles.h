#ifndef SHARED_SIM_TILES_H
#define SHARED_SIM_TILES_H

#include "shared/sim/chunk.h"

bool is_traversable(struct chunks *cnks, const struct point *p, uint8_t trav);
bool tile_is_traversable(enum tile t, uint8_t trav);
bool find_adj_tile(struct chunks *cnks, struct point *s, struct point *rp,
	struct rectangle *r, enum tile t, uint8_t et, uint8_t reject[4],
	bool (*pred)(enum tile t, uint8_t et));
enum tile get_tile_at(struct chunks *cnks, const struct point *p);
#endif
