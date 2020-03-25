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
	.tiles = {
		[tile_deep_water] = {
			"deep water",
		},
		[tile_water] = {
			"water",
			.next = tile_coral,
			.next_to = tile_wetland,
		},
		[tile_coral] = {
			"coral",
			.next = tile_water,
		},
		[tile_wetland] = {
			"wetland",
			.base = tile_wetland,
			.foundation = true,
			.next = tile_wetland_forest_young,
			.next_to = tile_water,
			.traversable = true,
		},
		[tile_wetland_forest_young] = {
			"sapling",
			.base = tile_wetland,
			.hardness = 50,
			.next = tile_wetland_forest,
			.traversable = true,
		},
		[tile_wetland_forest] = {
			"wetland forest",
			.base = tile_wetland,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_wetland_forest_old,
			.traversable = true,
		},
		[tile_wetland_forest_old] = {
			"dead tree",
			.base = tile_wetland,
			.hardness = 50,
			.next = tile_wetland,
			.traversable = true,
		},
		[tile_plain] = {
			"plain",
			.base = tile_dirt,
			.foundation = true,
			.next = tile_forest_young,
			.next_to = tile_forest,
			.traversable = true,
		},
		[tile_forest] = {
			"forest",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_forest_old,
			.traversable = true,
		},
		[tile_mountain] = {
			"mountain",
			.base = tile_rock_floor,
			.drop = et_resource_rock,
			.hardness = 255,
		},
		[tile_peak] = {
			"peak",
			.base = tile_rock_floor,
		},
		[tile_dirt] = {
			"dirt",
			.base = tile_dirt,
			.foundation = true,
			.next = tile_plain,
			.next_to = tile_plain,
			.traversable = true,
		},
		[tile_forest_young] = {
			"sapling",
			.base = tile_dirt,
			.hardness = 50,
			.next = tile_forest,
			.traversable = true,
		},
		[tile_forest_old] = {
			"dead tree",
			.base = tile_dirt,
			.hardness = 50,
			.next = tile_dirt,
			.traversable = true,
		},
		[tile_wood] = {
			"wood",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 50,
			.makeup = et_resource_wood,
		},
		[tile_stone] = {
			"stone",
			.base = tile_rock_floor,
			.drop = et_resource_rock,
			.hardness = 100,
			.makeup = et_resource_rock,
		},
		[tile_wood_floor] = {
			"wood floor",
			.base = tile_dirt,
			.foundation = true,
			.hardness = 10,
			.makeup = et_resource_wood,
			.traversable = true,
		},
		[tile_rock_floor] = {
			"rock floor",
			.base = tile_dirt,
			.foundation = true,
			.hardness = 10,
			.makeup = et_resource_rock,
			.traversable = true,
		},
		[tile_shrine] = {
			"shrine",
			.base = tile_dirt,
			.functional = true,
			.hardness = 300,
			.makeup = et_resource_wood,
		},
	},
};
