#ifndef SERVER_SIM_STOREHOUSE_H
#define SERVER_SIM_STOREHOUSE_H

#include "shared/sim/chunk.h"
#include "shared/sim/world.h"

struct storehouse_storage *get_storehouse_storage_at(struct chunks *cnks, const struct point *p);
struct storehouse_storage *nearest_storehouse(struct chunks *cnks, const struct point *p, uint32_t type);
bool storehouse_store(struct storehouse_storage *st, uint32_t type);
bool storehouse_take(struct storehouse_storage *st, uint32_t type);
bool storehouse_contains(struct storehouse_storage *st, uint32_t type);
void create_storehouse(struct world *w, const struct point *p);
void destroy_storehouse(struct world *w, const struct point *p);
#endif
