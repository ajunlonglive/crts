#include "shared/constants/globals.h"
#include "shared/sim/action.h"

static struct blueprint_block bldg_house_blueprint[] = {
	{ { -1, -1 }, tile_bldg },
	{ {  0, -1 }, tile_bldg },
	{ {  1, -1 }, tile_bldg },
	{ { -1,  0 }, tile_bldg },
	{ {  0,  0 }, tile_bldg },
	{ {  1,  0 }, tile_bldg },
	{ { -1,  1 }, tile_bldg },
	{ {  0,  1 }, tile_bldg },
	{ {  1,  1 }, tile_bldg },
};

const struct global_cfg_t gcfg = {
	.actions = {
		/*                  name         maxw   minw   diff.  satis.  */
		[at_none]       = { "nothing",      0,     0,     0,      0 },
		[at_move]       = { "move",      9999,     0,     1,    100 },
		[at_harvest]    = { "harvest",   9999,     0,    25,    100 },
		[at_build]      = { "build",     9999,     0,    25,    100 },
	},
	.ents = {
		[et_worker]        = { "worker", true  },
		[et_resource_wood] = { "wood",   false },
	},
	.blueprints = {
		[bldg_house] = { bldg_house_blueprint, { { -1, -1 }, 3, 3 }, 9, 15 },
	},
};
