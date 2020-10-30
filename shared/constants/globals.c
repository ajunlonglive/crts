#include "posix.h"

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
		[et_none] = { "ghost", .trav = 0xff },
		[et_worker] = {
			"elf",
			.animate = true,
			.corpse = et_elf_corpse,
			.hp = 100,
			.lifespan = 20000,
			.trav = trav_land,
		},
		[et_elf_corpse] = {
			"elf corpse",
			.hp = 300,
			.lifespan = 1000,
			.trav = 0xff
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
			.trav = trav_land,
		},
		[et_fish] = {
			"fish",
			.animate = true,
			.corpse = et_resource_meat,
			.group_size = 3,
			.hp = 50,
			.lifespan = 4000,
			.spawn_chance = 10000,
			.spawn_tile = tile_deep_water,
			.trav = trav_aquatic,
		},
		[et_resource_meat] = {
			"meat",
			.holdable = true,
			.lifespan = 1000,
			.trav = 0xff,
		},
		[et_resource_crop] = {
			"crop",
			.holdable = true,
			.lifespan = 1000,
			.trav = 0xff,
		},
		[et_resource_wood] = {
			"wood",
			.holdable = true,
			.lifespan = 5000,
			.trav = 0xff,
		},
		[et_resource_rock] = {
			"rock",
			.holdable = true,
			.lifespan = 5000,
			.trav = 0xff,
		},
		[et_vehicle_boat] = {
			"boat",
			.lifespan = 0,
			.trav = trav_aquatic,
		},
		[et_storehouse] = {
			"storehouse",
			.phantom = true,
		},
	},
	.tiles = {
		[tile_deep_water] = {
			"deep water",
			.trav_type = trav_aquatic,
		},
		[tile_water] = {
			"water",
			.next = tile_coral,
			.next_to = tile_wetland,
			.trav_type = trav_aquatic,
		},
		[tile_coral] = {
			"coral",
			.next = tile_water,
			.trav_type = trav_aquatic,
		},
		[tile_wetland] = {
			"wetland",
			.base = tile_wetland,
			.foundation = true,
			.next = tile_wetland_forest_young,
			.next_to = tile_water,
			.trav_type = trav_land,
		},
		[tile_wetland_forest_young] = {
			"sapling",
			.base = tile_wetland,
			.hardness = 50,
			.next = tile_wetland_forest,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_wetland_forest] = {
			"wetland forest",
			.base = tile_wetland,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_wetland_forest_old,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_wetland_forest_old] = {
			"dead tree",
			.base = tile_wetland,
			.hardness = 50,
			.next = tile_wetland,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_plain] = {
			"plain",
			.base = tile_dirt,
			.foundation = true,
			.next = tile_forest_young,
			.next_to = tile_forest_old,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_forest] = {
			"forest",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 100,
			.next = tile_forest_old,
			.trav_type = trav_land,
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
			.next_to = tile_dirt,
			.trav_type = trav_land,
		},
		[tile_forest_young] = {
			"sapling",
			.base = tile_dirt,
			.hardness = 50,
			.next = tile_forest,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_forest_old] = {
			"dead tree",
			.base = tile_dirt,
			.hardness = 50,
			.next = tile_dirt,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_wood] = {
			"wood",
			.base = tile_dirt,
			.drop = et_resource_wood,
			.hardness = 50,
			.makeup = et_resource_wood,
			.flamable = true,
			.build = blpt_frame,
		},
		[tile_stone] = {
			"stone",
			.base = tile_rock_floor,
			.drop = et_resource_rock,
			.hardness = 100,
			.makeup = et_resource_rock,
			.build = blpt_frame,
		},
		[tile_wood_floor] = {
			"wood floor",
			.base = tile_dirt,
			.foundation = true,
			.hardness = 10,
			.makeup = et_resource_wood,
			.trav_type = trav_land,
			.flamable = true,
			.build = blpt_rect,
		},
		[tile_rock_floor] = {
			"rock floor",
			.base = tile_dirt,
			.foundation = true,
			.hardness = 10,
			.makeup = et_resource_rock,
			.next = tile_dirt,
			.next_to = tile_plain,
			.trav_type = trav_land,
			.build = blpt_rect,
		},
		[tile_farmland_empty] = {
			"empty farmland",
			.base = tile_dirt,
			.foundation = true,
			.function = tfunc_dynamic,
			.trav_type = trav_land,
			.build = blpt_rect,
		},
		[tile_farmland_done] = {
			"farmland",
			.base = tile_farmland_empty,
			.drop = et_resource_crop,
			.foundation = true,
			.hardness = 25,
			.trav_type = trav_land,
			.flamable = true,
		},
		[tile_burning] = {
			"fire",
			.base = tile_burnt,
			.function = tfunc_dynamic,
			.trav_type = trav_land,
			.build = blpt_rect,
		},
		[tile_burnt] = {
			"ashes",
			.base = tile_dirt,
			.foundation = true,
			.trav_type = trav_land,
		},
		[tile_stream] = {
			"stream",
			.base = tile_dirt,
			.foundation = false,
			.trav_type = trav_land,
		},
		[tile_storehouse] = {
			"storehouse",
			.base = tile_dirt,
			.build = blpt_single,
			.foundation = false,
			.function = tfunc_storage,
			.hardness = 100,
			.makeup = et_resource_wood,
			.trav_type = trav_land,
		},
	},
	/* Fields ending in _rate specify the number of ticks before some event
	 * happens.  Fields ending in _chance specify a 1 in that number chance
	 * of an event happening.
	 */
	.misc = {
		.spawnable_ents = { et_deer, et_fish },
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
		.initial_spawn_amount = 800,
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
		.terrain_base_adj_grow_chance = 2000,
		/* If this number is 0, the chance of growing is
		 * terrain_base_not_adj_grow_chance
		 */
		.terrain_base_not_adj_grow_chance = 0x40000,
		/* when generating a new chunk, it is "aged"
		 * terrain_initial_age_multiplier * random() % terrain_initial_age_max
		 * times
		 */
		.terrain_initial_age_multiplier = 10,
		.terrain_initial_age_max = 100,

		.max_hunger = 2000,
		.get_hungry_chance = 10,
		.food_search_cooldown = 64,
	}
};
