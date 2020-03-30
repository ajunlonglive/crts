#ifndef __TERRAIN_H
#define __TERRAIN_H

#include <stdbool.h>

#include "shared/sim/ent.h"
#include "shared/sim/chunk.h"
#include "shared/types/geom.h"
#include "shared/types/hash.h"

struct chunk *get_chunk(struct chunks *cnks, const struct point *p);
struct chunk *get_chunk_at(struct chunks *cnks, const struct point *p);
bool find_tile(enum tile t, struct chunks *cnks, const struct circle *range,
	const struct point *start, struct point *p, struct hash *skip);
bool is_traversable(struct chunks *cnks, const struct point *p, enum ent_type et);
bool tile_is_traversable(enum tile t, enum ent_type et);
bool find_adj_tile(struct chunks *cnks, struct point *s, struct point *rp,
	struct circle *circ, enum tile t, enum ent_type et,
	bool (*pred)(enum tile t, enum ent_type et));
void update_tile(struct chunks *cnks, const struct point *p, enum tile t);
enum tile get_tile_at(struct chunks *cnks, const struct point *p);
void update_functional_tile(struct chunks *cnks, const struct point *p,
	enum tile t, uint16_t mot, uint32_t tick);
void touch_chunk(struct chunks *cnks, struct chunk *ck);
bool age_chunk(struct chunk *ck);
#endif
