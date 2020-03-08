#include "shared/constants/globals.h"
#include "shared/sim/action.h"

static struct blueprint_block bldg_block_blueprint[] = {
	{ {  0,  0 }, tile_bldg },
};

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

static struct blueprint_block bldg_star_blueprint[] = {
	{ { -1, -1 }, tile_bldg },
	{ {  1, -1 }, tile_bldg },
	{ {  0,  0 }, tile_bldg },
	{ { -1,  1 }, tile_bldg },
	{ {  1,  1 }, tile_bldg },
};

static struct blueprint_block bldg_tri_blueprint[] = {
	{ { -1, -1 }, tile_bldg },
	{ {  0,  0 }, tile_bldg },
	{ { -1,  0 }, tile_bldg },
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
		[at_fight]      = { "fight",     9999,     0,    25,    100 },
	},
	.ents = {
		[et_none]          = { "ghost",  false },
		[et_worker]        = { "worker", true  },
		[et_resource_wood] = { "wood",   false },
		[et_resource_rock] = { "rock",   false },
	},
	.blueprints = {
		[bldg_block] = { bldg_block_blueprint, { {  0,  0 }, 1, 1 }, 1,  2 },
		[bldg_house] = { bldg_house_blueprint, { { -1, -1 }, 3, 3 }, 9, 15 },
		[bldg_star]  = { bldg_star_blueprint,  { { -1, -1 }, 3, 3 }, 5, 10 },
		[bldg_tri]   = { bldg_tri_blueprint,   { { -1, -1 }, 3, 3 }, 6, 11 },
	},
	.harvestable = {
		[aht_forest]   = { 100, et_resource_wood, tile_dirt },
		[aht_mountain] = { 255, et_resource_rock, tile_dirt },
		[aht_bldg]     = { 100, et_resource_wood, tile_dirt },
	},
};
