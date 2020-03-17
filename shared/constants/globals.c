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
		[et_none]          = { "ghost",  false,    0 },
		[et_worker]        = { "worker", true,  5000 },
		[et_resource_wood] = { "wood",   false, 1000 },
		[et_resource_rock] = { "rock",   false, 1000 },
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
	.tiles = {
		[tile_deep_water]   = { "deep water", false, tile_deep_water },
		[tile_water]        = { "water",      false, tile_water },
		[tile_sand]         = { "sand",       true,  tile_sand },
		[tile_plain]        = { "plain",      true,  tile_forest_young },
		[tile_forest]       = { "forest",     true,  tile_forest_old },
		[tile_mountain]     = { "mountain",   false, tile_mountain },
		[tile_peak]         = { "peak",       false, tile_peak },
		[tile_dirt]         = { "dirt",       true,  tile_plain },
		[tile_forest_young] = { "sapling",    true,  tile_forest },
		[tile_forest_old]   = { "dead tree",  true,  tile_dirt },
		[tile_bldg]         = { "building",   false, tile_bldg },
	},
};
