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
		[at_carry]      = { "carry",     9999,     0,     1,    100 },
	},
	.ents = {
		[et_none] = { "ghost", },
		[et_worker] = {
			"elf",
			.animate = true,
			.corpse = et_elf_corpse,
			.hp = 100,
			.lifespan = 9000,
		},
		[et_elf_corpse] = {
			.hp = 300,
			.lifespan = 1000,
		},
		[et_deer] = {
			"deer",
			.animate = true,
			.corpse = et_resource_meat,
			.group_size = 3,
			.hp = 50,
			.lifespan = 4000,
			.spawn_chance = 10000,
			.spawn_tile = tile_plain,
		},
		[et_resource_meat] = {
			"meat",
			.holdable = true,
			.lifespan = 1000,
		},
		[et_resource_crop] = {
			"crop",
			.holdable = true,
			.lifespan = 1000,
		},
		[et_resource_wood] = {
			"wood",
			.holdable = true,
			.lifespan = 5000
		},
		[et_resource_rock] = {
			"rock",
			.holdable = true,
			.lifespan = 5000
		},
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
			.flamable = true,
		},
		[tile_wetland_forest] = {
			"wetland forest",
			.base = tile_wetland,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_wetland_forest_old,
			.traversable = true,
			.flamable = true,
		},
		[tile_wetland_forest_old] = {
			"dead tree",
			.base = tile_wetland,
			.hardness = 50,
			.next = tile_wetland,
			.traversable = true,
			.flamable = true,
		},
		[tile_plain] = {
			"plain",
			.base = tile_dirt,
			.foundation = true,
			.next = tile_forest_young,
			.next_to = tile_forest,
			.traversable = true,
			.flamable = true,
		},
		[tile_forest] = {
			"forest",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_forest_old,
			.traversable = true,
			.flamable = true,
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
			.flamable = true,
		},
		[tile_forest_old] = {
			"dead tree",
			.base = tile_dirt,
			.hardness = 50,
			.next = tile_dirt,
			.traversable = true,
			.flamable = true,
		},
		[tile_wood] = {
			"wood",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 50,
			.makeup = et_resource_wood,
			.flamable = true,
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
			.flamable = true,
		},
		[tile_rock_floor] = {
			"rock floor",
			.base = tile_dirt,
			.foundation = true,
			.hardness = 10,
			.makeup = et_resource_rock,
			.next = tile_dirt,
			.next_to = tile_plain,
			.traversable = true,
		},
		[tile_shrine] = {
			"shrine",
			.base = tile_dirt,
			.functional = true,
			.hardness = 300,
			.makeup = et_resource_wood,
		},
		[tile_farmland_empty] = {
			"empty farmland",
			.base = tile_dirt,
			.foundation = true,
			.functional = true,
			.traversable = true,
		},
		[tile_farmland_done] = {
			"farmland",
			.base = tile_dirt,
			.drop = et_resource_crop,
			.foundation = true,
			.hardness = 25,
			.traversable = true,
			.flamable = true,
		},
		[tile_burning] = {
			"fire",
			.base = tile_burnt,
			.functional = true,
			.traversable = true,
		},
		[tile_burnt] = {
			"ashes",
			.base = tile_dirt,
			.foundation = true,
			.traversable = true,
		},
	},
	/* Fields ending in _rate specify the number of ticks before some event
	 * happens.  Fields ending in _chance specify a 1 in that number chance
	 * of an event happening.
	 */
	.misc = {
		.spawnable_ents = { et_deer },
		/* Delay before a shrine consumes a resource and spawns an elf */
		.shrine_spawn_rate = 64,
		/* radius of circle a shrine looks for food in */
		.shrine_range = 5,
		/* time it takes to grow */
		.farm_grow_rate = 256,
		/* damage taken by ents in fire */
		.fire_damage = 50,
		/* time before fire can spread */
		.fire_spread_rate = 10,
		/* chance of spreading */
		.fire_spread_chance = 10,
		/* once spreading, per tile chance of igniting */
		.fire_spread_ignite_chance = 2,
		/* random % this number produces starting x and y spawn coords */
		.initial_spawn_range = 100,
		/* amount of starting elves */
		.initial_spawn_amount = 100,
		/* chance an idle ent will move randomly */
		.meander_chance = 75,
		/* maximum amount an ent can age over its lifespan before dying */
		.max_over_age = 1000,

		/* when growing terrain, first adjacent tiles that match the
		 * current tiles .next_to are counted.
		 *
		 * If this number is greater than 0, the chance of growing is
		 * terrain_base_adj_grow_chance / adjacent count
		 */
		.terrain_base_adj_grow_chance = 4000,
		/* If this number is 0, the chance of growing is
		 * terrain_base_not_adj_grow_chance
		 */
		.terrain_base_not_adj_grow_chance = 0,
		/* when generating a new chunk, it is "aged"
		 * terrain_initial_age_multiplier * random() % terrain_initial_age_max
		 * times
		 */
		.terrain_initial_age_multiplier = 10,
		.terrain_initial_age_max = 100,
	}
};
