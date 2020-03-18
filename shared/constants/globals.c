#include "shared/constants/globals.h"
#include "shared/sim/action.h"

const struct global_cfg_t gcfg = {
	.actions = {
		/* TODO: remove unused fields from this */
		/*                  name         maxw   minw   diff.  satis.  */
		[at_none]       = { "nothing",      0,     0,     0,      0 },
		[at_move]       = { "move",      9999,     0,     1,    100 },
		[at_harvest]    = { "harvest",   9999,     0,     1,    100 },
		[at_build]      = { "build",     9999,     0,     1,    100 },
		[at_fight]      = { "fight",     9999,     0,     1,    100 },
	},
	.ents = {
		[et_none]          = { "ghost",  false,    0 },
		[et_worker]        = { "worker", true,  5000 },
		[et_resource_wood] = { "wood",   false, 1000 },
		[et_resource_rock] = { "rock",   false, 1000 },
	},
	.harvestable = {
		[aht_forest]   = { 100, et_resource_wood, tile_dirt },
		[aht_mountain] = { 255, et_resource_rock, tile_dirt },
		[aht_bldg]     = { 100, et_resource_wood, tile_dirt },
	},
	.tiles = {
		[tile_deep_water] = {
			"deep water",
		},
		[tile_water] = {
			"water",
			.next = tile_water
		},
		[tile_sand] = {
			"sand",
			.traversable = true,
			.foundation = true,
		},
		[tile_plain] = {
			"plain",
			.traversable = true,
			.foundation = true,
			.next = tile_forest_young
		},
		[tile_forest] = {
			"forest",
			.traversable = true,
			.next = tile_forest_old
		},
		[tile_mountain] = {
			"mountain",
		},
		[tile_peak] = {
			"peak",
		},
		[tile_dirt] = {
			"dirt",
			.traversable = true,
			.foundation = true,
			.next = tile_plain
		},
		[tile_forest_young] = {
			"sapling",
			.traversable = true,
			.next = tile_forest
		},
		[tile_forest_old] = {
			"dead tree",
			.traversable = true,
			.next = tile_dirt
		},
		[tile_wood] = {
			"wood",
			.next = tile_wood,
			.makeup = et_resource_wood,
		},
		[tile_stone] = {
			"stone",
			.next = tile_stone,
			.makeup = et_resource_rock,
		},
	},
};
