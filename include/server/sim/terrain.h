#ifndef __TERRAIN_H
#define __TERRAIN_H

#include <stdbool.h>

#include "shared/sim/chunk.h"
#include "shared/types/geom.h"
#include "shared/types/hash.h"

struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
struct chunk *get_chunk_at(struct chunks *cnks, const struct point *p);
bool find_tile(enum tile t, struct chunks *cnks, const struct circle *range,
	const struct point *start, struct point *p, struct hash *skip);
bool is_traversable(struct chunks *cnks, const struct point *p);
bool tile_is_traversable(enum tile t);
void update_tile(struct chunks *cnks, const struct point *p, enum tile t);
void update_tile_at(struct chunks *cnks, struct chunk *ck, int x, int y, enum tile t);
enum tile get_tile_at(struct chunks *cnks, const struct point *p);
#endif
