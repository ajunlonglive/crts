#ifndef SHARED_CONSANTS_BLUEPRINTS_H
#define SHARED_CONSANTS_BLUEPRINTS_H

#include "shared/sim/chunk.h"

#define BLUEPRINT_LEN 32

struct blueprint_block {
	struct point p;
	enum tile t;
};

struct blueprint {
	char *name;
	const struct blueprint_block* blocks;
	size_t len;
};

enum building {
	bldg_wood_block       = 0 << 1,
	bldg_stone_block      = 1 << 1,
	bldg_wood_wall        = 2 << 1,
	bldg_stone_wall       = 3 << 1,
	buildings_count       = 4 * 2,

	bldg_rotate = 0x1,
};

extern const struct blueprint blueprints[buildings_count];
#endif
