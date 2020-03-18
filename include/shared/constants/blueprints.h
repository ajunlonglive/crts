#ifndef SHARED_CONSANTS_BLUEPRINTS_H
#define SHARED_CONSANTS_BLUEPRINTS_H

#include "shared/sim/chunk.h"

#define BLUEPRINT_LEN 32

struct blueprint_block {
	struct point p;
	enum tile t;
};

struct blueprint {
	const struct blueprint_block* blocks;
	size_t len;
};

enum building {
	bldg_wood_block,
	bldg_stone_block,
	bldg_wood_wall_horiz,
	bldg_stone_wall_horiz,
	buildings_count,
};

extern const struct blueprint blueprints[buildings_count];
#endif
