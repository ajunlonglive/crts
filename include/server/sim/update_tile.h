#ifndef SERVER_SIM_UPDATE_TILE_H
#define SERVER_SIM_UPDATE_TILE_H

#include "shared/sim/world.h"

void update_tile(struct world *w, const struct point *p, enum tile t);
float update_tile_height(struct world *w, const struct point *p, float delta);
void update_functional_tile(struct world *w, const struct point *p,
	enum tile t, uint16_t mot, uint32_t tick);
void touch_chunk(struct chunks *cnks, struct chunk *ck);
#endif
